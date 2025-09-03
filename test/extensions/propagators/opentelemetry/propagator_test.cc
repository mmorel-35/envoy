#include "source/extensions/propagators/opentelemetry/propagator.h"

#include "source/common/http/header_map_impl.h"
#include "source/common/tracing/trace_context_impl.h"
#include "test/mocks/api/mocks.h"

#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace OpenTelemetry {
namespace {

class OpenTelemetryPropagatorTest : public ::testing::Test {
protected:
  void SetUp() override {
    headers_ = Http::RequestHeaderMapImpl::create();
    trace_context_ = std::make_unique<Tracing::TraceContextImpl>(*headers_);
    
    // Setup mock API for environment variable testing
    ON_CALL(api_, getEnv("OTEL_PROPAGATORS")).WillByDefault(Return(absl::nullopt));
  }

  void addW3CHeaders(absl::string_view traceparent, absl::string_view tracestate = "",
                     absl::string_view baggage = "") {
    headers_->addCopy("traceparent", traceparent);
    if (!tracestate.empty()) {
      headers_->addCopy("tracestate", tracestate);
    }
    if (!baggage.empty()) {
      headers_->addCopy("baggage", baggage);
    }
  }

  void addB3Headers(absl::string_view trace_id, absl::string_view span_id,
                    absl::string_view parent_span_id = "", absl::string_view sampled = "") {
    headers_->addCopy("x-b3-traceid", trace_id);
    headers_->addCopy("x-b3-spanid", span_id);
    if (!parent_span_id.empty()) {
      headers_->addCopy("x-b3-parentspanid", parent_span_id);
    }
    if (!sampled.empty()) {
      headers_->addCopy("x-b3-sampled", sampled);
    }
  }

  void addB3SingleHeader(absl::string_view b3_value) {
    headers_->addCopy("b3", b3_value);
  }

  Http::RequestHeaderMapPtr headers_;
  std::unique_ptr<Tracing::TraceContextImpl> trace_context_;
  testing::NiceMock<Api::MockApi> api_;
  
  // Test data
  const std::string valid_traceparent = "00-1234567890abcdef1234567890abcdef-fedcba0987654321-01";
  const std::string valid_tracestate = "vendor=value,other=data";
  const std::string valid_baggage = "key1=value1,key2=value2";
  const std::string valid_trace_id = "1234567890abcdef1234567890abcdef";
  const std::string valid_span_id = "fedcba0987654321";
};

// Test presence detection
TEST_F(OpenTelemetryPropagatorTest, IsNotPresentWhenEmpty) {
  EXPECT_FALSE(Propagator::isPresent(*trace_context_));
}

TEST_F(OpenTelemetryPropagatorTest, IsPresentWithW3CHeaders) {
  addW3CHeaders(valid_traceparent);
  EXPECT_TRUE(Propagator::isPresent(*trace_context_));
}

TEST_F(OpenTelemetryPropagatorTest, IsPresentWithB3Headers) {
  addB3Headers(valid_trace_id, valid_span_id);
  EXPECT_TRUE(Propagator::isPresent(*trace_context_));
}

// Test default extraction behavior (W3C first, B3 fallback)
TEST_F(OpenTelemetryPropagatorTest, ExtractW3CFirst) {
  addW3CHeaders(valid_traceparent, valid_tracestate, valid_baggage);
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_EQ(context.format(), TraceFormat::W3C);
  EXPECT_TRUE(context.hasW3CContext());
  EXPECT_EQ(context.getTraceId(), "1234567890abcdef1234567890abcdef");
  EXPECT_EQ(context.getSpanId(), "fedcba0987654321");
  EXPECT_TRUE(context.isSampled());
}

TEST_F(OpenTelemetryPropagatorTest, ExtractB3Fallback) {
  addB3Headers(valid_trace_id, valid_span_id, "1111222233334444", "1");
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_EQ(context.format(), TraceFormat::B3);
  EXPECT_TRUE(context.hasB3Context());
  EXPECT_EQ(context.getTraceId(), valid_trace_id);
  EXPECT_EQ(context.getSpanId(), valid_span_id);
  EXPECT_TRUE(context.isSampled());
}

TEST_F(OpenTelemetryPropagatorTest, ExtractW3CPriority) {
  // Add both W3C and B3 headers - W3C should take priority
  addW3CHeaders(valid_traceparent);
  addB3Headers("different0trace0id", "different00span", "", "1");
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_EQ(context.format(), TraceFormat::W3C);
  EXPECT_EQ(context.getTraceId(), "1234567890abcdef1234567890abcdef");
}

// Test configuration-based extraction
TEST_F(OpenTelemetryPropagatorTest, ExtractWithTraceContextConfig) {
  addW3CHeaders(valid_traceparent);
  
  Config config = Propagator::createConfig({PropagatorType::TraceContext});
  auto result = Propagator::extract(*trace_context_, config);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  EXPECT_EQ(result.value().format(), TraceFormat::W3C);
}

TEST_F(OpenTelemetryPropagatorTest, ExtractWithB3Config) {
  addB3Headers(valid_trace_id, valid_span_id, "", "1");
  
  Config config = Propagator::createConfig({PropagatorType::B3Multi});
  auto result = Propagator::extract(*trace_context_, config);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  EXPECT_EQ(result.value().format(), TraceFormat::B3);
}

TEST_F(OpenTelemetryPropagatorTest, ExtractWithB3SingleConfig) {
  addB3SingleHeader("1234567890abcdef-fedcba0987654321-1");
  
  Config config = Propagator::createConfig({PropagatorType::B3});
  auto result = Propagator::extract(*trace_context_, config);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  EXPECT_EQ(result.value().format(), TraceFormat::B3);
}

// Test propagator priority order in configuration
TEST_F(OpenTelemetryPropagatorTest, ExtractRespectsPropagatorOrder) {
  // Add both formats
  addW3CHeaders(valid_traceparent);
  addB3Headers("different0trace0id", "different00span", "", "1");
  
  // B3 first in config should extract B3 despite W3C being present
  Config config = Propagator::createConfig({PropagatorType::B3Multi, PropagatorType::TraceContext});
  auto result = Propagator::extract(*trace_context_, config);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  EXPECT_EQ(result.value().format(), TraceFormat::B3);
  EXPECT_EQ(result.value().getTraceId(), "different0trace0id");
}

// Test "none" propagator behavior
TEST_F(OpenTelemetryPropagatorTest, ExtractWithNonePropagator) {
  addW3CHeaders(valid_traceparent);
  addB3Headers(valid_trace_id, valid_span_id);
  
  Config config = Propagator::createConfig({PropagatorType::None});
  auto result = Propagator::extract(*trace_context_, config);
  EXPECT_FALSE(result.ok()); // Should fail to extract anything
}

// Test injection behavior - all configured propagators
TEST_F(OpenTelemetryPropagatorTest, InjectMultiplePropagators) {
  auto w3c_traceparent = W3C::TraceParent("00", valid_trace_id, valid_span_id, "01");
  auto w3c_context = W3C::TraceContext(w3c_traceparent);
  CompositeTraceContext composite(w3c_context);
  
  Config config = Propagator::createConfig({PropagatorType::TraceContext, PropagatorType::B3Multi});
  auto status = Propagator::inject(composite, *trace_context_, config);
  ASSERT_TRUE(status.ok()) << status.message();
  
  // Should inject both W3C and B3 headers
  EXPECT_FALSE(headers_->get_("traceparent").empty());
  EXPECT_FALSE(headers_->get_("x-b3-traceid").empty());
  EXPECT_FALSE(headers_->get_("x-b3-spanid").empty());
  EXPECT_FALSE(headers_->get_("x-b3-sampled").empty());
}

TEST_F(OpenTelemetryPropagatorTest, InjectWithBaggage) {
  auto w3c_traceparent = W3C::TraceParent("00", valid_trace_id, valid_span_id, "01");
  auto w3c_context = W3C::TraceContext(w3c_traceparent);
  CompositeTraceContext composite(w3c_context);
  
  Config config = Propagator::createConfig({PropagatorType::TraceContext, PropagatorType::Baggage});
  auto status = Propagator::inject(composite, *trace_context_, config);
  ASSERT_TRUE(status.ok()) << status.message();
  
  EXPECT_FALSE(headers_->get_("traceparent").empty());
  // Baggage header should be present (even if empty initially)
}

// Test "none" propagator clears headers
TEST_F(OpenTelemetryPropagatorTest, InjectNoneClearsHeaders) {
  // Pre-populate headers
  addW3CHeaders(valid_traceparent, valid_tracestate, valid_baggage);
  addB3Headers(valid_trace_id, valid_span_id, "", "1");
  
  auto w3c_traceparent = W3C::TraceParent("00", valid_trace_id, valid_span_id, "01");
  auto w3c_context = W3C::TraceContext(w3c_traceparent);
  CompositeTraceContext composite(w3c_context);
  
  Config config = Propagator::createConfig({PropagatorType::None});
  auto status = Propagator::inject(composite, *trace_context_, config);
  ASSERT_TRUE(status.ok()) << status.message();
  
  // All propagation headers should be cleared
  EXPECT_TRUE(headers_->get_("traceparent").empty());
  EXPECT_TRUE(headers_->get_("tracestate").empty());
  EXPECT_TRUE(headers_->get_("baggage").empty());
  EXPECT_TRUE(headers_->get_("x-b3-traceid").empty());
  EXPECT_TRUE(headers_->get_("x-b3-spanid").empty());
  EXPECT_TRUE(headers_->get_("b3").empty());
}

// Test baggage extraction and injection
TEST_F(OpenTelemetryPropagatorTest, ExtractBaggage) {
  addW3CHeaders(valid_traceparent, "", valid_baggage);
  
  auto result = Propagator::extractBaggage(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& baggage = result.value();
  EXPECT_TRUE(baggage.hasValues());
  EXPECT_EQ(baggage.getValue("key1"), "value1");
  EXPECT_EQ(baggage.getValue("key2"), "value2");
}

TEST_F(OpenTelemetryPropagatorTest, InjectBaggage) {
  W3C::Baggage w3c_baggage;
  w3c_baggage.set(W3C::BaggageMember("test_key", "test_value"));
  CompositeBaggage composite_baggage(w3c_baggage);
  
  auto status = Propagator::injectBaggage(composite_baggage, *trace_context_);
  ASSERT_TRUE(status.ok()) << status.message();
  
  std::string baggage_header = headers_->get_("baggage");
  EXPECT_FALSE(baggage_header.empty());
  EXPECT_THAT(baggage_header, testing::HasSubstr("test_key=test_value"));
}

// Test configuration creation from proto and environment
TEST_F(OpenTelemetryPropagatorTest, CreateConfigFromProto) {
  envoy::config::trace::v3::OpenTelemetryConfig otel_config;
  otel_config.add_propagators("tracecontext");
  otel_config.add_propagators("b3multi");
  otel_config.add_propagators("baggage");
  
  auto config = Propagator::createConfig(otel_config, api_);
  EXPECT_EQ(config.propagators.size(), 3);
  EXPECT_EQ(config.propagators[0], PropagatorType::TraceContext);
  EXPECT_EQ(config.propagators[1], PropagatorType::B3Multi);
  EXPECT_EQ(config.propagators[2], PropagatorType::Baggage);
}

TEST_F(OpenTelemetryPropagatorTest, CreateConfigFromEnvironment) {
  envoy::config::trace::v3::OpenTelemetryConfig otel_config; // Empty proto config
  
  // Mock environment variable
  ON_CALL(api_, getEnv("OTEL_PROPAGATORS"))
      .WillByDefault(Return(absl::make_optional("tracecontext,b3,baggage")));
  
  auto config = Propagator::createConfig(otel_config, api_);
  EXPECT_EQ(config.propagators.size(), 3);
  EXPECT_EQ(config.propagators[0], PropagatorType::TraceContext);
  EXPECT_EQ(config.propagators[1], PropagatorType::B3);
  EXPECT_EQ(config.propagators[2], PropagatorType::Baggage);
}

TEST_F(OpenTelemetryPropagatorTest, CreateConfigDefaultsToTraceContext) {
  envoy::config::trace::v3::OpenTelemetryConfig otel_config; // Empty
  // No environment variable
  
  auto config = Propagator::createConfig(otel_config, api_);
  EXPECT_EQ(config.propagators.size(), 1);
  EXPECT_EQ(config.propagators[0], PropagatorType::TraceContext);
}

TEST_F(OpenTelemetryPropagatorTest, CreateConfigEnvironmentOverridesProto) {
  envoy::config::trace::v3::OpenTelemetryConfig otel_config;
  otel_config.add_propagators("tracecontext"); // Proto config
  
  // Environment should override
  ON_CALL(api_, getEnv("OTEL_PROPAGATORS"))
      .WillByDefault(Return(absl::make_optional("b3,baggage")));
  
  auto config = Propagator::createConfig(otel_config, api_);
  EXPECT_EQ(config.propagators.size(), 2);
  EXPECT_EQ(config.propagators[0], PropagatorType::B3);
  EXPECT_EQ(config.propagators[1], PropagatorType::Baggage);
}

// Test PropagatorService IoC implementation
class PropagatorServiceTest : public OpenTelemetryPropagatorTest {
protected:
  void SetUp() override {
    OpenTelemetryPropagatorTest::SetUp();
    
    Config config = Propagator::createConfig({PropagatorType::TraceContext, PropagatorType::B3Multi});
    service_ = std::make_unique<PropagatorService>(config);
  }

  std::unique_ptr<PropagatorService> service_;
};

TEST_F(PropagatorServiceTest, ServiceIsPresent) {
  addW3CHeaders(valid_traceparent);
  EXPECT_TRUE(service_->isPresent(*trace_context_));
  
  headers_->clear();
  addB3Headers(valid_trace_id, valid_span_id);
  EXPECT_TRUE(service_->isPresent(*trace_context_));
  
  headers_->clear();
  EXPECT_FALSE(service_->isPresent(*trace_context_));
}

TEST_F(PropagatorServiceTest, ServiceExtract) {
  addW3CHeaders(valid_traceparent);
  
  auto result = service_->extract(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  EXPECT_EQ(result.value().format(), TraceFormat::W3C);
  EXPECT_EQ(result.value().getTraceId(), "1234567890abcdef1234567890abcdef");
}

TEST_F(PropagatorServiceTest, ServiceInject) {
  auto w3c_traceparent = W3C::TraceParent("00", valid_trace_id, valid_span_id, "01");
  auto w3c_context = W3C::TraceContext(w3c_traceparent);
  CompositeTraceContext composite(w3c_context);
  
  auto status = service_->inject(composite, *trace_context_);
  ASSERT_TRUE(status.ok()) << status.message();
  
  // Should inject both W3C and B3 (per service config)
  EXPECT_FALSE(headers_->get_("traceparent").empty());
  EXPECT_FALSE(headers_->get_("x-b3-traceid").empty());
}

TEST_F(PropagatorServiceTest, ServiceBaggageOperations) {
  addW3CHeaders(valid_traceparent, "", valid_baggage);
  
  // Test getBaggageValue
  std::string value = service_->getBaggageValue(*trace_context_, "key1");
  EXPECT_EQ(value, "value1");
  
  // Test setBaggageValue
  bool result = service_->setBaggageValue(*trace_context_, "new_key", "new_value");
  EXPECT_TRUE(result);
  
  std::string new_value = service_->getBaggageValue(*trace_context_, "new_key");
  EXPECT_EQ(new_value, "new_value");
}

TEST_F(PropagatorServiceTest, ServiceCreateFromTracerData) {
  auto result = service_->createFromTracerData(
      valid_trace_id, valid_span_id, "parent_span_id", true, "vendor=data");
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_EQ(context.getTraceId(), valid_trace_id);
  EXPECT_EQ(context.getSpanId(), valid_span_id);
  EXPECT_TRUE(context.isSampled());
}

// Test TracingHelper for backward compatibility
TEST_F(OpenTelemetryPropagatorTest, TracingHelperExtract) {
  addW3CHeaders(valid_traceparent, valid_tracestate);
  
  auto context = TracingHelper::extractForTracer(*trace_context_);
  ASSERT_TRUE(context.has_value());
  
  EXPECT_EQ(context->getTraceId(), "1234567890abcdef1234567890abcdef");
  EXPECT_EQ(context->getSpanId(), "fedcba0987654321");
  EXPECT_TRUE(context->isSampled());
}

TEST_F(OpenTelemetryPropagatorTest, TracingHelperWithConfig) {
  addB3Headers(valid_trace_id, valid_span_id, "", "1");
  
  Config config = Propagator::createConfig({PropagatorType::B3Multi});
  auto result = TracingHelper::extractWithConfig(*trace_context_, config);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  EXPECT_EQ(result.value().format(), TraceFormat::B3);
}

TEST_F(OpenTelemetryPropagatorTest, TracingHelperCreateFromData) {
  auto result = TracingHelper::createFromTracerData(
      valid_trace_id, valid_span_id, "parent_span", true, "vendor=data");
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_EQ(context.getTraceId(), valid_trace_id);
  EXPECT_TRUE(context.isSampled());
}

// Test BaggageHelper
TEST_F(OpenTelemetryPropagatorTest, BaggageHelperOperations) {
  addW3CHeaders(valid_traceparent, "", valid_baggage);
  
  // Test getBaggageValue
  std::string value = BaggageHelper::getBaggageValue(*trace_context_, "key1");
  EXPECT_EQ(value, "value1");
  
  // Test setBaggageValue
  bool result = BaggageHelper::setBaggageValue(*trace_context_, "new_key", "new_value");
  EXPECT_TRUE(result);
  
  // Test getAllBaggage
  auto all_baggage = BaggageHelper::getAllBaggage(*trace_context_);
  EXPECT_FALSE(all_baggage.empty());
  EXPECT_EQ(all_baggage["key1"], "value1");
  EXPECT_EQ(all_baggage["new_key"], "new_value");
  
  // Test hasBaggage
  EXPECT_TRUE(BaggageHelper::hasBaggage(*trace_context_));
}

// OpenTelemetry Specification Compliance Tests
class OpenTelemetrySpecComplianceTest : public OpenTelemetryPropagatorTest {
protected:
  void testPropagatorConfigOrder(const std::vector<std::string>& propagator_names,
                                const std::vector<PropagatorType>& expected_types) {
    envoy::config::trace::v3::OpenTelemetryConfig otel_config;
    for (const auto& name : propagator_names) {
      otel_config.add_propagators(name);
    }
    
    auto config = Propagator::createConfig(otel_config, api_);
    ASSERT_EQ(config.propagators.size(), expected_types.size());
    
    for (size_t i = 0; i < expected_types.size(); ++i) {
      EXPECT_EQ(config.propagators[i], expected_types[i]) 
        << "Propagator at index " << i << " doesn't match expected type";
    }
  }
};

// Test propagator name parsing compliance
TEST_F(OpenTelemetrySpecComplianceTest, PropagatorNameCompliance) {
  // Test standard propagator names
  EXPECT_EQ(Propagator::stringToPropagatorType("tracecontext").value(), PropagatorType::TraceContext);
  EXPECT_EQ(Propagator::stringToPropagatorType("baggage").value(), PropagatorType::Baggage);
  EXPECT_EQ(Propagator::stringToPropagatorType("b3").value(), PropagatorType::B3);
  EXPECT_EQ(Propagator::stringToPropagatorType("b3multi").value(), PropagatorType::B3Multi);
  EXPECT_EQ(Propagator::stringToPropagatorType("none").value(), PropagatorType::None);
  
  // Test case insensitivity
  EXPECT_EQ(Propagator::stringToPropagatorType("TRACECONTEXT").value(), PropagatorType::TraceContext);
  EXPECT_EQ(Propagator::stringToPropagatorType("TraceContext").value(), PropagatorType::TraceContext);
  EXPECT_EQ(Propagator::stringToPropagatorType("B3MULTI").value(), PropagatorType::B3Multi);
  EXPECT_EQ(Propagator::stringToPropagatorType("NONE").value(), PropagatorType::None);
}

// Test default behavior compliance
TEST_F(OpenTelemetrySpecComplianceTest, DefaultBehaviorCompliance) {
  // When no configuration is provided, should default to "tracecontext"
  envoy::config::trace::v3::OpenTelemetryConfig empty_config;
  auto config = Propagator::createConfig(empty_config, api_);
  
  ASSERT_EQ(config.propagators.size(), 1);
  EXPECT_EQ(config.propagators[0], PropagatorType::TraceContext);
}

// Test OTEL_PROPAGATORS environment variable precedence
TEST_F(OpenTelemetrySpecComplianceTest, EnvironmentVariablePrecedence) {
  // Set up environment variable
  EXPECT_CALL(api_, getEnv("OTEL_PROPAGATORS"))
    .WillOnce(Return(std::string("b3,baggage,tracecontext")));
  
  // Configure different propagators in proto
  envoy::config::trace::v3::OpenTelemetryConfig otel_config;
  otel_config.add_propagators("tracecontext");
  otel_config.add_propagators("b3multi");
  
  auto config = Propagator::createConfig(otel_config, api_);
  
  // Environment variable should take precedence
  ASSERT_EQ(config.propagators.size(), 3);
  EXPECT_EQ(config.propagators[0], PropagatorType::B3);
  EXPECT_EQ(config.propagators[1], PropagatorType::Baggage);
  EXPECT_EQ(config.propagators[2], PropagatorType::TraceContext);
}

// Test "none" propagator behavior
TEST_F(OpenTelemetrySpecComplianceTest, NonePropagatorCompliance) {
  // "none" should disable all propagation regardless of other propagators
  testPropagatorConfigOrder(
    {"tracecontext", "none", "b3"},
    {PropagatorType::None}
  );
  
  testPropagatorConfigOrder(
    {"none"},
    {PropagatorType::None}
  );
  
  // Test that "none" clears all headers
  addW3CHeaders(valid_traceparent, "vendor=data", valid_baggage);
  addB3Headers("1234567890abcdef", "fedcba0987654321", "", "1");
  headers_->addCopy("b3", "1234567890abcdef-fedcba0987654321-1");
  
  Propagator::Config config;
  config.propagators = {PropagatorType::None};
  
  CompositeTraceContext dummy_context;
  auto status = Propagator::inject(dummy_context, *trace_context_, config);
  EXPECT_TRUE(status.ok());
  
  // All headers should be cleared
  EXPECT_FALSE(trace_context_->getByKey("traceparent").has_value());
  EXPECT_FALSE(trace_context_->getByKey("tracestate").has_value());
  EXPECT_FALSE(trace_context_->getByKey("baggage").has_value());
  EXPECT_FALSE(trace_context_->getByKey("x-b3-traceid").has_value());
  EXPECT_FALSE(trace_context_->getByKey("b3").has_value());
}

// Test extraction order compliance
TEST_F(OpenTelemetrySpecComplianceTest, ExtractionOrderCompliance) {
  // Add both W3C and B3 headers
  addW3CHeaders(valid_traceparent);
  addB3Headers("1234567890abcdef", "fedcba0987654321", "", "1");
  
  // Test W3C first
  Propagator::Config w3c_first_config;
  w3c_first_config.propagators = {PropagatorType::TraceContext, PropagatorType::B3};
  
  auto result1 = Propagator::extract(*trace_context_, w3c_first_config);
  ASSERT_TRUE(result1.ok());
  EXPECT_EQ(result1.value().format(), TraceFormat::W3C);
  
  // Test B3 first
  Propagator::Config b3_first_config;
  b3_first_config.propagators = {PropagatorType::B3, PropagatorType::TraceContext};
  
  auto result2 = Propagator::extract(*trace_context_, b3_first_config);
  ASSERT_TRUE(result2.ok());
  EXPECT_EQ(result2.value().format(), TraceFormat::B3);
}

// Test B3 format distinction compliance
TEST_F(OpenTelemetrySpecComplianceTest, B3FormatDistinctionCompliance) {
  // Test B3 single header format
  headers_->addCopy("b3", "1234567890abcdef-fedcba0987654321-1");
  
  // Configure b3 (single header)
  Propagator::Config b3_config;
  b3_config.propagators = {PropagatorType::B3};
  
  auto result1 = Propagator::extract(*trace_context_, b3_config);
  ASSERT_TRUE(result1.ok());
  EXPECT_EQ(result1.value().format(), TraceFormat::B3);
  
  // Clear and add multiple headers
  headers_->remove("b3");
  addB3Headers("1234567890abcdef", "fedcba0987654321", "", "1");
  
  // Configure b3multi (multiple headers)
  Propagator::Config b3multi_config;
  b3multi_config.propagators = {PropagatorType::B3Multi};
  
  auto result2 = Propagator::extract(*trace_context_, b3multi_config);
  ASSERT_TRUE(result2.ok());
  EXPECT_EQ(result2.value().format(), TraceFormat::B3);
}

// Test injection behavior compliance
TEST_F(OpenTelemetrySpecComplianceTest, InjectionBehaviorCompliance) {
  // Create a W3C context
  auto w3c_result = W3C::Propagator::createRoot(valid_trace_id, valid_span_id, true);
  ASSERT_TRUE(w3c_result.ok());
  CompositeTraceContext composite_context(w3c_result.value());
  
  // Configure multiple propagators
  Propagator::Config multi_config;
  multi_config.propagators = {PropagatorType::TraceContext, PropagatorType::B3Multi, PropagatorType::Baggage};
  
  auto status = Propagator::inject(composite_context, *trace_context_, multi_config);
  ASSERT_TRUE(status.ok());
  
  // All configured formats should be injected
  EXPECT_TRUE(trace_context_->getByKey("traceparent").has_value());
  EXPECT_TRUE(trace_context_->getByKey("x-b3-traceid").has_value());
  EXPECT_TRUE(trace_context_->getByKey("x-b3-spanid").has_value());
  // Baggage header injection depends on presence of baggage data
}

// Test duplicate propagator handling
TEST_F(OpenTelemetrySpecComplianceTest, DuplicatePropagatorHandling) {
  // Duplicates should be removed while preserving order
  testPropagatorConfigOrder(
    {"tracecontext", "b3", "tracecontext", "baggage", "b3"},
    {PropagatorType::TraceContext, PropagatorType::B3, PropagatorType::Baggage}
  );
}

// Test unknown propagator handling
TEST_F(OpenTelemetrySpecComplianceTest, UnknownPropagatorHandling) {
  // Unknown propagators should be ignored
  testPropagatorConfigOrder(
    {"tracecontext", "unknown", "b3", "invalid", "baggage"},
    {PropagatorType::TraceContext, PropagatorType::B3, PropagatorType::Baggage}
  );
}

// Test propagator service IoC compliance
TEST_F(OpenTelemetrySpecComplianceTest, PropagatorServiceCompliance) {
  Propagator::Config config;
  config.propagators = {PropagatorType::TraceContext, PropagatorType::B3Multi};
  
  PropagatorService service(config);
  
  // Test extraction
  addW3CHeaders(valid_traceparent);
  auto result = service.extract(*trace_context_);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result.value().format(), TraceFormat::W3C);
  
  // Test injection
  auto status = service.inject(result.value(), *trace_context_);
  EXPECT_TRUE(status.ok());
  
  // Both formats should be present
  EXPECT_TRUE(trace_context_->getByKey("traceparent").has_value());
  EXPECT_TRUE(trace_context_->getByKey("x-b3-traceid").has_value());
}

} // namespace
} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
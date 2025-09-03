#include "source/extensions/propagators/opentelemetry/propagator.h"

#include "source/extensions/propagators/w3c/propagator.h"
#include "source/extensions/propagators/b3/propagator.h"
#include "source/common/tracing/trace_context_impl.h"

#include "test/mocks/tracing/mocks.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace OpenTelemetry {
namespace {

using ::testing::Return;
using ::testing::_;

class OpenTelemetryPropagatorTest : public ::testing::Test {
protected:
  void SetUp() override {
    trace_context_ = std::make_unique<Tracing::TraceContextImpl>(headers_);
  }

  Http::TestRequestHeaderMapImpl headers_;
  std::unique_ptr<Tracing::TraceContext> trace_context_;
};

// Test W3C format extraction and injection
TEST_F(OpenTelemetryPropagatorTest, ExtractW3CFormat) {
  // Set up W3C headers
  headers_.set("traceparent", "00-0af7651916cd43dd8448eb211c80319c-b7ad6b7169203331-01");
  headers_.set("tracestate", "congo=t61rcWkgMzE");
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok());
  
  auto context = result.value();
  EXPECT_EQ(context.format(), TraceFormat::W3C);
  EXPECT_EQ(context.getTraceId(), "0af7651916cd43dd8448eb211c80319c");
  EXPECT_EQ(context.getSpanId(), "b7ad6b7169203331");
  EXPECT_TRUE(context.isSampled());
  EXPECT_EQ(context.getTraceState(), "congo=t61rcWkgMzE");
}

TEST_F(OpenTelemetryPropagatorTest, ExtractB3Format) {
  // Set up B3 multiple headers
  headers_.set("x-b3-traceid", "0af7651916cd43dd8448eb211c80319c");
  headers_.set("x-b3-spanid", "b7ad6b7169203331");
  headers_.set("x-b3-sampled", "1");
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok());
  
  auto context = result.value();
  EXPECT_EQ(context.format(), TraceFormat::B3);
  EXPECT_EQ(context.getTraceId(), "0af7651916cd43dd8448eb211c80319c");
  EXPECT_EQ(context.getSpanId(), "b7ad6b7169203331");
  EXPECT_TRUE(context.isSampled());
  EXPECT_TRUE(context.getTraceState().empty()); // B3 doesn't have tracestate
}

TEST_F(OpenTelemetryPropagatorTest, ExtractB3SingleHeader) {
  // Set up B3 single header
  headers_.set("b3", "0af7651916cd43dd8448eb211c80319c-b7ad6b7169203331-1");
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok());
  
  auto context = result.value();
  EXPECT_EQ(context.format(), TraceFormat::B3);
  EXPECT_EQ(context.getTraceId(), "0af7651916cd43dd8448eb211c80319c");
  EXPECT_EQ(context.getSpanId(), "b7ad6b7169203331");
  EXPECT_TRUE(context.isSampled());
}

TEST_F(OpenTelemetryPropagatorTest, ExtractPriorityW3COverB3) {
  // Set up both W3C and B3 headers - W3C should take priority
  headers_.set("traceparent", "00-0af7651916cd43dd8448eb211c80319c-b7ad6b7169203331-01");
  headers_.set("x-b3-traceid", "different-trace-id");
  headers_.set("x-b3-spanid", "different-span-id");
  headers_.set("x-b3-sampled", "0");
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok());
  
  auto context = result.value();
  EXPECT_EQ(context.format(), TraceFormat::W3C);
  EXPECT_EQ(context.getTraceId(), "0af7651916cd43dd8448eb211c80319c");
  EXPECT_EQ(context.getSpanId(), "b7ad6b7169203331");
  EXPECT_TRUE(context.isSampled());
}

TEST_F(OpenTelemetryPropagatorTest, ExtractNoHeaders) {
  // No trace headers
  auto result = Propagator::extract(*trace_context_);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kNotFound);
}

TEST_F(OpenTelemetryPropagatorTest, InjectW3CFormat) {
  // Create W3C context
  auto w3c_result = W3C::Propagator::createRoot("0af7651916cd43dd8448eb211c80319c", "b7ad6b7169203331", true);
  ASSERT_TRUE(w3c_result.ok());
  
  CompositeTraceContext composite_context(w3c_result.value());
  
  // Clear headers and inject
  headers_.clear();
  auto inject_result = Propagator::inject(composite_context, *trace_context_);
  ASSERT_TRUE(inject_result.ok());
  
  // Check injected headers
  auto traceparent = headers_.get("traceparent");
  ASSERT_TRUE(traceparent.has_value());
  EXPECT_THAT(traceparent.value(), testing::HasSubstr("0af7651916cd43dd8448eb211c80319c"));
  EXPECT_THAT(traceparent.value(), testing::HasSubstr("b7ad6b7169203331"));
  EXPECT_THAT(traceparent.value(), testing::HasSubstr("-01")); // sampled flag
}

TEST_F(OpenTelemetryPropagatorTest, InjectB3Format) {
  // Create B3 context
  auto trace_id = B3::TraceId::fromHexString("0af7651916cd43dd8448eb211c80319c");
  auto span_id = B3::SpanId::fromHexString("b7ad6b7169203331");
  ASSERT_TRUE(trace_id.ok());
  ASSERT_TRUE(span_id.ok());
  
  B3::TraceContext b3_ctx(trace_id.value(), span_id.value(), absl::nullopt, B3::SamplingState::SAMPLED);
  CompositeTraceContext composite_context(b3_ctx);
  
  // Configure for B3 injection
  Propagator::Config config;
  config.injection_format = Propagator::InjectionFormat::B3_ONLY;
  
  // Clear headers and inject
  headers_.clear();
  auto inject_result = Propagator::inject(composite_context, *trace_context_, config);
  ASSERT_TRUE(inject_result.ok());
  
  // Check injected headers
  auto trace_id_header = headers_.get("x-b3-traceid");
  auto span_id_header = headers_.get("x-b3-spanid");
  auto sampled_header = headers_.get("x-b3-sampled");
  
  ASSERT_TRUE(trace_id_header.has_value());
  ASSERT_TRUE(span_id_header.has_value());
  ASSERT_TRUE(sampled_header.has_value());
  
  EXPECT_EQ(trace_id_header.value(), "0af7651916cd43dd8448eb211c80319c");
  EXPECT_EQ(span_id_header.value(), "b7ad6b7169203331");
  EXPECT_EQ(sampled_header.value(), "1");
}

TEST_F(OpenTelemetryPropagatorTest, InjectBothFormats) {
  // Create context
  auto w3c_result = W3C::Propagator::createRoot("0af7651916cd43dd8448eb211c80319c", "b7ad6b7169203331", true);
  ASSERT_TRUE(w3c_result.ok());
  
  CompositeTraceContext composite_context(w3c_result.value());
  
  // Configure for both formats injection
  Propagator::Config config;
  config.injection_format = Propagator::InjectionFormat::BOTH;
  
  // Clear headers and inject
  headers_.clear();
  auto inject_result = Propagator::inject(composite_context, *trace_context_, config);
  ASSERT_TRUE(inject_result.ok());
  
  // Check both W3C and B3 headers are present
  EXPECT_TRUE(headers_.get("traceparent").has_value());
  EXPECT_TRUE(headers_.get("x-b3-traceid").has_value());
  EXPECT_TRUE(headers_.get("x-b3-spanid").has_value());
  EXPECT_TRUE(headers_.get("x-b3-sampled").has_value());
}

TEST_F(OpenTelemetryPropagatorTest, FormatConversion) {
  // Create W3C context
  auto w3c_result = W3C::Propagator::createRoot("0af7651916cd43dd8448eb211c80319c", "b7ad6b7169203331", true);
  ASSERT_TRUE(w3c_result.ok());
  
  CompositeTraceContext w3c_context(w3c_result.value());
  
  // Convert to B3 format
  auto b3_result = w3c_context.convertTo(TraceFormat::B3);
  ASSERT_TRUE(b3_result.ok());
  
  auto b3_context = b3_result.value();
  EXPECT_EQ(b3_context.format(), TraceFormat::B3);
  EXPECT_EQ(b3_context.getTraceId(), "0af7651916cd43dd8448eb211c80319c");
  EXPECT_EQ(b3_context.getSpanId(), "b7ad6b7169203331");
  EXPECT_TRUE(b3_context.isSampled());
}

TEST_F(OpenTelemetryPropagatorTest, CreateChildContext) {
  // Create parent context
  auto parent_result = Propagator::createRoot("0af7651916cd43dd8448eb211c80319c", "b7ad6b7169203331", true);
  ASSERT_TRUE(parent_result.ok());
  
  auto parent_context = parent_result.value();
  
  // Create child context
  auto child_result = Propagator::createChild(parent_context, "c7ad6b7169203332");
  ASSERT_TRUE(child_result.ok());
  
  auto child_context = child_result.value();
  EXPECT_EQ(child_context.getTraceId(), "0af7651916cd43dd8448eb211c80319c"); // Same trace ID
  EXPECT_EQ(child_context.getSpanId(), "c7ad6b7169203332"); // New span ID
  EXPECT_EQ(child_context.getParentSpanId(), "b7ad6b7169203331"); // Parent's span ID
  EXPECT_TRUE(child_context.isSampled());
}

// Test baggage functionality
TEST_F(OpenTelemetryPropagatorTest, ExtractBaggage) {
  // Set up W3C headers with baggage
  headers_.set("traceparent", "00-0af7651916cd43dd8448eb211c80319c-b7ad6b7169203331-01");
  headers_.set("baggage", "userId=alice,sessionId=123456");
  
  auto baggage_result = Propagator::extractBaggage(*trace_context_);
  ASSERT_TRUE(baggage_result.ok());
  
  auto baggage = baggage_result.value();
  EXPECT_FALSE(baggage.isEmpty());
  EXPECT_EQ(baggage.getValue("userId"), "alice");
  EXPECT_EQ(baggage.getValue("sessionId"), "123456");
  EXPECT_EQ(baggage.getValue("nonexistent"), "");
}

TEST_F(OpenTelemetryPropagatorTest, InjectBaggage) {
  // Create baggage
  CompositeBaggage baggage;
  baggage.setValue("userId", "alice");
  baggage.setValue("sessionId", "123456");
  
  // Clear headers and inject
  headers_.clear();
  auto inject_result = Propagator::injectBaggage(baggage, *trace_context_);
  ASSERT_TRUE(inject_result.ok());
  
  // Check injected baggage header
  auto baggage_header = headers_.get("baggage");
  ASSERT_TRUE(baggage_header.has_value());
  EXPECT_THAT(baggage_header.value(), testing::HasSubstr("userId=alice"));
  EXPECT_THAT(baggage_header.value(), testing::HasSubstr("sessionId=123456"));
}

// Test TracingHelper functionality
TEST_F(OpenTelemetryPropagatorTest, TracingHelperExtract) {
  // Set up W3C headers
  headers_.set("traceparent", "00-0af7651916cd43dd8448eb211c80319c-b7ad6b7169203331-01");
  
  auto context = TracingHelper::extractForTracer(*trace_context_);
  ASSERT_TRUE(context.has_value());
  
  EXPECT_EQ(context.value().format(), TraceFormat::W3C);
  EXPECT_EQ(context.value().getTraceId(), "0af7651916cd43dd8448eb211c80319c");
  EXPECT_EQ(context.value().getSpanId(), "b7ad6b7169203331");
  EXPECT_TRUE(context.value().isSampled());
}

TEST_F(OpenTelemetryPropagatorTest, TracingHelperExtractWithFallback) {
  // Set up only B3 headers
  headers_.set("x-b3-traceid", "0af7651916cd43dd8448eb211c80319c");
  headers_.set("x-b3-spanid", "b7ad6b7169203331");
  headers_.set("x-b3-sampled", "1");
  
  TracingHelper::TracerConfig config;
  config.preferred_format = TraceFormat::W3C;
  config.enable_format_fallback = true;
  
  auto context = TracingHelper::extractForTracer(*trace_context_, config);
  ASSERT_TRUE(context.has_value());
  
  // Should fallback to B3
  EXPECT_EQ(context.value().format(), TraceFormat::B3);
  EXPECT_EQ(context.value().getTraceId(), "0af7651916cd43dd8448eb211c80319c");
  EXPECT_EQ(context.value().getSpanId(), "b7ad6b7169203331");
  EXPECT_TRUE(context.value().isSampled());
}

TEST_F(OpenTelemetryPropagatorTest, TracingHelperPropagationHeaderPresent) {
  // No headers
  EXPECT_FALSE(TracingHelper::propagationHeaderPresent(*trace_context_));
  
  // W3C headers
  headers_.set("traceparent", "00-0af7651916cd43dd8448eb211c80319c-b7ad6b7169203331-01");
  EXPECT_TRUE(TracingHelper::propagationHeaderPresent(*trace_context_));
  
  // Clear and add B3 headers
  headers_.clear();
  headers_.set("x-b3-traceid", "0af7651916cd43dd8448eb211c80319c");
  EXPECT_TRUE(TracingHelper::propagationHeaderPresent(*trace_context_));
}

// Test BaggageHelper functionality
TEST_F(OpenTelemetryPropagatorTest, BaggageHelperGetValue) {
  // Set up baggage header
  headers_.set("baggage", "userId=alice,sessionId=123456");
  
  EXPECT_EQ(BaggageHelper::getBaggageValue(*trace_context_, "userId"), "alice");
  EXPECT_EQ(BaggageHelper::getBaggageValue(*trace_context_, "sessionId"), "123456");
  EXPECT_EQ(BaggageHelper::getBaggageValue(*trace_context_, "nonexistent"), "");
}

TEST_F(OpenTelemetryPropagatorTest, BaggageHelperSetValue) {
  // Start with empty headers
  headers_.clear();
  
  EXPECT_TRUE(BaggageHelper::setBaggageValue(*trace_context_, "userId", "alice"));
  EXPECT_TRUE(BaggageHelper::setBaggageValue(*trace_context_, "sessionId", "123456"));
  
  // Check that baggage was set
  auto baggage_header = headers_.get("baggage");
  ASSERT_TRUE(baggage_header.has_value());
  EXPECT_THAT(baggage_header.value(), testing::HasSubstr("userId=alice"));
  EXPECT_THAT(baggage_header.value(), testing::HasSubstr("sessionId=123456"));
}

TEST_F(OpenTelemetryPropagatorTest, BaggageHelperGetAllBaggage) {
  // Set up baggage header
  headers_.set("baggage", "userId=alice,sessionId=123456,env=production");
  
  auto all_baggage = BaggageHelper::getAllBaggage(*trace_context_);
  EXPECT_EQ(all_baggage.size(), 3);
  EXPECT_EQ(all_baggage["userId"], "alice");
  EXPECT_EQ(all_baggage["sessionId"], "123456");
  EXPECT_EQ(all_baggage["env"], "production");
}

TEST_F(OpenTelemetryPropagatorTest, BaggageHelperHasBaggage) {
  // No baggage
  EXPECT_FALSE(BaggageHelper::hasBaggage(*trace_context_));
  
  // Add baggage
  headers_.set("baggage", "userId=alice");
  EXPECT_TRUE(BaggageHelper::hasBaggage(*trace_context_));
}

} // namespace
} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
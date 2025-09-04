#include "source/extensions/propagators/w3c/propagator.h"
#include "source/extensions/tracers/opentelemetry/span_context_extractor.h"
#include "source/extensions/tracers/zipkin/span_context_extractor.h"

#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace {

// Test that the W3C propagator integration works with different tracers
class W3CPropagatorIntegrationTest : public testing::Test {
protected:
  void SetUp() override {
    // Common W3C trace context values for testing
    version_ = "00";
    trace_id_ = "0af7651916cd43dd8448eb211c80319c";
    span_id_ = "b7ad6b7169203331";
    trace_flags_ = "01";
    traceparent_value_ = fmt::format("{}-{}-{}-{}", version_, trace_id_, span_id_, trace_flags_);
  }

  std::string version_;
  std::string trace_id_;
  std::string span_id_;
  std::string trace_flags_;
  std::string traceparent_value_;
};

// Test that OpenTelemetry span context extractor works with W3C propagator
TEST_F(W3CPropagatorIntegrationTest, OpenTelemetryExtractorWithW3CPropagator) {
  Tracing::TestTraceContextImpl request_headers{{"traceparent", traceparent_value_}};

  // Test that W3C propagator can extract the context
  auto w3c_result = Propagators::W3C::Propagator::extract(request_headers);
  EXPECT_TRUE(w3c_result.ok()) << w3c_result.status().message();

  if (w3c_result.ok()) {
    const auto& w3c_context = w3c_result.value();
    EXPECT_EQ(w3c_context.traceParent().traceId(), trace_id_);
    EXPECT_EQ(w3c_context.traceParent().parentId(), span_id_);
    EXPECT_TRUE(w3c_context.traceParent().isSampled());
  }

  // Test that OpenTelemetry extractor works with the same headers
  OpenTelemetry::SpanContextExtractor otel_extractor(request_headers);
  EXPECT_TRUE(otel_extractor.propagationHeaderPresent());

  auto otel_result = otel_extractor.extractSpanContext();
  EXPECT_TRUE(otel_result.ok()) << otel_result.status().message();

  if (otel_result.ok()) {
    const auto& otel_context = otel_result.value();
    EXPECT_EQ(otel_context.traceId(), trace_id_);
    EXPECT_EQ(otel_context.spanId(), span_id_);
    EXPECT_TRUE(otel_context.sampled());
  }
}

// Test that Zipkin span context extractor works with W3C propagator for fallback
TEST_F(W3CPropagatorIntegrationTest, ZipkinExtractorWithW3CFallback) {
  Tracing::TestTraceContextImpl request_headers{{"traceparent", traceparent_value_}};

  // Test that W3C propagator can extract the context
  auto w3c_result = Propagators::W3C::Propagator::extract(request_headers);
  EXPECT_TRUE(w3c_result.ok()) << w3c_result.status().message();

  // Test that Zipkin extractor with W3C fallback enabled works
  Zipkin::SpanContextExtractor zipkin_extractor(request_headers, true /* w3c_fallback_enabled */);
  
  // Should extract sampled flag from W3C headers
  auto sampled_result = zipkin_extractor.extractSampled();
  EXPECT_TRUE(sampled_result.has_value());
  EXPECT_TRUE(sampled_result.value());

  // Should extract full span context from W3C headers
  auto zipkin_result = zipkin_extractor.extractSpanContext(true);
  EXPECT_TRUE(zipkin_result.second); // Successfully extracted

  if (zipkin_result.second) {
    const auto& zipkin_context = zipkin_result.first;
    // Note: In Zipkin conversion, the 128-bit trace ID is split into high and low parts
    EXPECT_TRUE(zipkin_context.sampled());
  }
}

// Test that W3C headers can be injected and then extracted by different tracers
TEST_F(W3CPropagatorIntegrationTest, W3CRoundTripCompatibility) {
  // Create a W3C trace context
  auto w3c_context_result = Propagators::W3C::Propagator::createRoot(trace_id_, span_id_, true);
  EXPECT_TRUE(w3c_context_result.ok()) << w3c_context_result.status().message();

  if (!w3c_context_result.ok()) {
    return;
  }

  // Inject the context into headers
  Tracing::TestTraceContextImpl headers{};
  Propagators::W3C::Propagator::inject(w3c_context_result.value(), headers);

  // Verify the headers were set correctly
  auto traceparent_header = headers.get("traceparent");
  EXPECT_TRUE(traceparent_header.has_value());
  EXPECT_EQ(traceparent_header.value(), traceparent_value_);

  // Test that OpenTelemetry can extract from the injected headers
  OpenTelemetry::SpanContextExtractor otel_extractor(headers);
  auto otel_result = otel_extractor.extractSpanContext();
  EXPECT_TRUE(otel_result.ok());

  if (otel_result.ok()) {
    EXPECT_EQ(otel_result.value().traceId(), trace_id_);
    EXPECT_EQ(otel_result.value().spanId(), span_id_);
    EXPECT_TRUE(otel_result.value().sampled());
  }

  // Test that Zipkin can extract from the injected headers (with fallback)
  Zipkin::SpanContextExtractor zipkin_extractor(headers, true);
  auto zipkin_result = zipkin_extractor.extractSpanContext(true);
  EXPECT_TRUE(zipkin_result.second);
  EXPECT_TRUE(zipkin_result.first.sampled());
}

} // namespace
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
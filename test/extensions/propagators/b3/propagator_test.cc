#include "source/extensions/propagators/b3/propagator.h"

#include "source/common/http/header_map_impl.h"
#include "source/common/tracing/trace_context_impl.h"

#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace B3 {
namespace {

class B3PropagatorTest : public ::testing::Test {
protected:
  void SetUp() override {
    headers_ = Http::RequestHeaderMapImpl::create();
    trace_context_ = std::make_unique<Tracing::TraceContextImpl>(*headers_);
  }

  void addB3MultipleHeaders(absl::string_view trace_id, absl::string_view span_id,
                          absl::string_view parent_span_id = "",
                          absl::string_view sampled = "") {
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
  
  // Test data
  const std::string valid_trace_id = "1234567890abcdef1234567890abcdef";
  const std::string valid_span_id = "1234567890abcdef";
  const std::string valid_parent_span_id = "fedcba0987654321";
};

// Test presence detection
TEST_F(B3PropagatorTest, IsNotPresentWhenEmpty) {
  EXPECT_FALSE(Propagator::isPresent(*trace_context_));
}

TEST_F(B3PropagatorTest, IsPresentWithMultipleHeaders) {
  addB3MultipleHeaders(valid_trace_id, valid_span_id);
  EXPECT_TRUE(Propagator::isPresent(*trace_context_));
}

TEST_F(B3PropagatorTest, IsPresentWithSingleHeader) {
  addB3SingleHeader("1234567890abcdef-fedcba0987654321-1");
  EXPECT_TRUE(Propagator::isPresent(*trace_context_));
}

TEST_F(B3PropagatorTest, IsPresentWithPartialHeaders) {
  headers_->addCopy("x-b3-traceid", valid_trace_id);
  EXPECT_TRUE(Propagator::isPresent(*trace_context_));
  
  headers_->clear();
  headers_->addCopy("x-b3-spanid", valid_span_id);
  EXPECT_TRUE(Propagator::isPresent(*trace_context_));
}

// Test multiple headers extraction - valid cases
TEST_F(B3PropagatorTest, ExtractMultipleHeadersMinimal) {
  addB3MultipleHeaders(valid_trace_id, valid_span_id);
  
  auto result = Propagator::extractMultipleHeaders(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_TRUE(context.isValid());
  EXPECT_EQ(context.traceId().toHexString(), valid_trace_id);
  EXPECT_EQ(context.spanId().toHexString(), valid_span_id);
  EXPECT_FALSE(context.hasParentSpanId());
  EXPECT_EQ(context.samplingState(), SamplingState::NOT_SAMPLED);
}

TEST_F(B3PropagatorTest, ExtractMultipleHeadersComplete) {
  addB3MultipleHeaders(valid_trace_id, valid_span_id, valid_parent_span_id, "1");
  
  auto result = Propagator::extractMultipleHeaders(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_TRUE(context.isValid());
  EXPECT_EQ(context.traceId().toHexString(), valid_trace_id);
  EXPECT_EQ(context.spanId().toHexString(), valid_span_id);
  EXPECT_TRUE(context.hasParentSpanId());
  EXPECT_EQ(context.parentSpanId().toHexString(), valid_parent_span_id);
  EXPECT_EQ(context.samplingState(), SamplingState::SAMPLED);
}

// Test 64-bit trace ID support
TEST_F(B3PropagatorTest, ExtractMultipleHeaders64BitTraceId) {
  const std::string trace_id_64bit = "1234567890abcdef";
  addB3MultipleHeaders(trace_id_64bit, valid_span_id);
  
  auto result = Propagator::extractMultipleHeaders(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_TRUE(context.isValid());
  EXPECT_EQ(context.traceId().toHexString(), trace_id_64bit);
  EXPECT_FALSE(context.traceId().is128Bit());
}

// Test sampling state variations per B3 spec
TEST_F(B3PropagatorTest, ExtractSamplingStates) {
  // Test "0" (not sampled)
  addB3MultipleHeaders(valid_trace_id, valid_span_id, "", "0");
  auto result = Propagator::extractMultipleHeaders(*trace_context_);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result.value().samplingState(), SamplingState::NOT_SAMPLED);
  
  // Test "1" (sampled)
  headers_->clear();
  addB3MultipleHeaders(valid_trace_id, valid_span_id, "", "1");
  result = Propagator::extractMultipleHeaders(*trace_context_);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result.value().samplingState(), SamplingState::SAMPLED);
  
  // Test "d" (debug)
  headers_->clear();
  addB3MultipleHeaders(valid_trace_id, valid_span_id, "", "d");
  result = Propagator::extractMultipleHeaders(*trace_context_);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result.value().samplingState(), SamplingState::DEBUG);
  
  // Test legacy "true"
  headers_->clear();
  addB3MultipleHeaders(valid_trace_id, valid_span_id, "", "true");
  result = Propagator::extractMultipleHeaders(*trace_context_);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result.value().samplingState(), SamplingState::SAMPLED);
  
  // Test legacy "false"
  headers_->clear();
  addB3MultipleHeaders(valid_trace_id, valid_span_id, "", "false");
  result = Propagator::extractMultipleHeaders(*trace_context_);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result.value().samplingState(), SamplingState::NOT_SAMPLED);
}

// Test case insensitivity for header names (B3 spec requirement)
TEST_F(B3PropagatorTest, ExtractCaseInsensitiveHeaders) {
  headers_->addCopy("X-B3-TraceId", valid_trace_id);
  headers_->addCopy("X-B3-SpanId", valid_span_id);
  headers_->addCopy("X-B3-Sampled", "1");
  
  auto result = Propagator::extractMultipleHeaders(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_TRUE(context.isValid());
  EXPECT_EQ(context.samplingState(), SamplingState::SAMPLED);
}

// Test multiple headers extraction - error cases
TEST_F(B3PropagatorTest, ExtractMultipleHeadersMissingTraceId) {
  headers_->addCopy("x-b3-spanid", valid_span_id);
  
  auto result = Propagator::extractMultipleHeaders(*trace_context_);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(result.status().message(), testing::HasSubstr("Required"));
}

TEST_F(B3PropagatorTest, ExtractMultipleHeadersMissingSpanId) {
  headers_->addCopy("x-b3-traceid", valid_trace_id);
  
  auto result = Propagator::extractMultipleHeaders(*trace_context_);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(result.status().message(), testing::HasSubstr("Required"));
}

// Test zero ID rejection (B3 spec requirement)
TEST_F(B3PropagatorTest, ExtractRejectsZeroTraceId) {
  addB3MultipleHeaders("0000000000000000", valid_span_id);
  
  auto result = Propagator::extractMultipleHeaders(*trace_context_);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("zero"));
}

TEST_F(B3PropagatorTest, ExtractRejectsZeroSpanId) {
  addB3MultipleHeaders(valid_trace_id, "0000000000000000");
  
  auto result = Propagator::extractMultipleHeaders(*trace_context_);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("zero"));
}

// Test single header extraction - valid cases
TEST_F(B3PropagatorTest, ExtractSingleHeaderMinimal) {
  addB3SingleHeader("1234567890abcdef-fedcba0987654321");
  
  auto result = Propagator::extractSingleHeader(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_TRUE(context.isValid());
  EXPECT_EQ(context.traceId().toHexString(), "1234567890abcdef");
  EXPECT_EQ(context.spanId().toHexString(), "fedcba0987654321");
  EXPECT_FALSE(context.hasParentSpanId());
  EXPECT_EQ(context.samplingState(), SamplingState::NOT_SAMPLED);
}

TEST_F(B3PropagatorTest, ExtractSingleHeaderWithSampling) {
  addB3SingleHeader("1234567890abcdef-fedcba0987654321-1");
  
  auto result = Propagator::extractSingleHeader(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_TRUE(context.isValid());
  EXPECT_EQ(context.samplingState(), SamplingState::SAMPLED);
}

TEST_F(B3PropagatorTest, ExtractSingleHeaderComplete) {
  addB3SingleHeader("1234567890abcdef-fedcba0987654321-1-1111222233334444");
  
  auto result = Propagator::extractSingleHeader(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_TRUE(context.isValid());
  EXPECT_TRUE(context.hasParentSpanId());
  EXPECT_EQ(context.parentSpanId().toHexString(), "1111222233334444");
  EXPECT_EQ(context.samplingState(), SamplingState::SAMPLED);
}

// Test 128-bit trace ID in single header
TEST_F(B3PropagatorTest, ExtractSingleHeader128BitTraceId) {
  addB3SingleHeader("1234567890abcdef1234567890abcdef-fedcba0987654321-1");
  
  auto result = Propagator::extractSingleHeader(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_TRUE(context.isValid());
  EXPECT_TRUE(context.traceId().is128Bit());
  EXPECT_EQ(context.traceId().toHexString(), "1234567890abcdef1234567890abcdef");
}

// Test debug sampling in single header
TEST_F(B3PropagatorTest, ExtractSingleHeaderDebugSampling) {
  addB3SingleHeader("1234567890abcdef-fedcba0987654321-d");
  
  auto result = Propagator::extractSingleHeader(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_EQ(context.samplingState(), SamplingState::DEBUG);
}

// Test single header extraction - error cases
TEST_F(B3PropagatorTest, ExtractSingleHeaderMissing) {
  auto result = Propagator::extractSingleHeader(*trace_context_);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("not found"));
}

TEST_F(B3PropagatorTest, ExtractSingleHeaderEmpty) {
  addB3SingleHeader("");
  
  auto result = Propagator::extractSingleHeader(*trace_context_);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("empty"));
}

TEST_F(B3PropagatorTest, ExtractSingleHeaderMalformed) {
  // Missing span ID
  addB3SingleHeader("1234567890abcdef");
  auto result = Propagator::extractSingleHeader(*trace_context_);
  EXPECT_FALSE(result.ok());
  
  // Invalid trace ID
  addB3SingleHeader("invalid-fedcba0987654321");
  result = Propagator::extractSingleHeader(*trace_context_);
  EXPECT_FALSE(result.ok());
  
  // Too many parts
  addB3SingleHeader("1234567890abcdef-fedcba0987654321-1-parent-extra");
  result = Propagator::extractSingleHeader(*trace_context_);
  EXPECT_FALSE(result.ok());
}

// Test format detection and priority
TEST_F(B3PropagatorTest, ExtractPrefersMultipleHeaders) {
  // Add both formats
  addB3MultipleHeaders(valid_trace_id, valid_span_id, "", "1");
  addB3SingleHeader("fedcba0987654321-1234567890abcdef-0");
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  // Should prefer multiple headers format
  const auto& context = result.value();
  EXPECT_EQ(context.traceId().toHexString(), valid_trace_id);
  EXPECT_EQ(context.spanId().toHexString(), valid_span_id);
  EXPECT_EQ(context.samplingState(), SamplingState::SAMPLED);
}

TEST_F(B3PropagatorTest, ExtractFallsBackToSingleHeader) {
  addB3SingleHeader("1234567890abcdef-fedcba0987654321-1");
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_EQ(context.traceId().toHexString(), "1234567890abcdef");
  EXPECT_EQ(context.spanId().toHexString(), "fedcba0987654321");
  EXPECT_EQ(context.samplingState(), SamplingState::SAMPLED);
}

// Test injection - multiple headers format
TEST_F(B3PropagatorTest, InjectMultipleHeaders) {
  auto trace_id = TraceId::fromHexString(valid_trace_id).value();
  auto span_id = SpanId::fromHexString(valid_span_id).value();
  auto parent_span_id = SpanId::fromHexString(valid_parent_span_id).value();
  
  TraceContext context(trace_id, span_id, parent_span_id, SamplingState::SAMPLED);
  
  auto status = Propagator::injectMultipleHeaders(context, *trace_context_);
  ASSERT_TRUE(status.ok()) << status.message();
  
  // Verify headers are set correctly
  EXPECT_EQ(headers_->get_("x-b3-traceid"), valid_trace_id);
  EXPECT_EQ(headers_->get_("x-b3-spanid"), valid_span_id);
  EXPECT_EQ(headers_->get_("x-b3-parentspanid"), valid_parent_span_id);
  EXPECT_EQ(headers_->get_("x-b3-sampled"), "1");
}

TEST_F(B3PropagatorTest, InjectMultipleHeadersMinimal) {
  auto trace_id = TraceId::fromHexString(valid_trace_id).value();
  auto span_id = SpanId::fromHexString(valid_span_id).value();
  
  TraceContext context(trace_id, span_id);
  
  auto status = Propagator::injectMultipleHeaders(context, *trace_context_);
  ASSERT_TRUE(status.ok()) << status.message();
  
  // Verify headers are set correctly
  EXPECT_EQ(headers_->get_("x-b3-traceid"), valid_trace_id);
  EXPECT_EQ(headers_->get_("x-b3-spanid"), valid_span_id);
  EXPECT_EQ(headers_->get_("x-b3-sampled"), "0");
  // Parent span ID should not be set
  EXPECT_TRUE(headers_->get_("x-b3-parentspanid").empty());
}

// Test injection - single header format
TEST_F(B3PropagatorTest, InjectSingleHeaderMinimal) {
  auto trace_id = TraceId::fromHexString("1234567890abcdef").value();
  auto span_id = SpanId::fromHexString("fedcba0987654321").value();
  
  TraceContext context(trace_id, span_id);
  
  auto status = Propagator::injectSingleHeader(context, *trace_context_);
  ASSERT_TRUE(status.ok()) << status.message();
  
  EXPECT_EQ(headers_->get_("b3"), "1234567890abcdef-fedcba0987654321");
}

TEST_F(B3PropagatorTest, InjectSingleHeaderWithSampling) {
  auto trace_id = TraceId::fromHexString("1234567890abcdef").value();
  auto span_id = SpanId::fromHexString("fedcba0987654321").value();
  
  TraceContext context(trace_id, span_id, SpanId(), SamplingState::SAMPLED);
  
  auto status = Propagator::injectSingleHeader(context, *trace_context_);
  ASSERT_TRUE(status.ok()) << status.message();
  
  EXPECT_EQ(headers_->get_("b3"), "1234567890abcdef-fedcba0987654321-1");
}

TEST_F(B3PropagatorTest, InjectSingleHeaderComplete) {
  auto trace_id = TraceId::fromHexString("1234567890abcdef").value();
  auto span_id = SpanId::fromHexString("fedcba0987654321").value();
  auto parent_span_id = SpanId::fromHexString("1111222233334444").value();
  
  TraceContext context(trace_id, span_id, parent_span_id, SamplingState::DEBUG);
  
  auto status = Propagator::injectSingleHeader(context, *trace_context_);
  ASSERT_TRUE(status.ok()) << status.message();
  
  EXPECT_EQ(headers_->get_("b3"), "1234567890abcdef-fedcba0987654321-d-1111222233334444");
}

// Test default injection behavior (should prefer multiple headers)
TEST_F(B3PropagatorTest, InjectDefaultsToMultipleHeaders) {
  auto trace_id = TraceId::fromHexString(valid_trace_id).value();
  auto span_id = SpanId::fromHexString(valid_span_id).value();
  
  TraceContext context(trace_id, span_id, SpanId(), SamplingState::SAMPLED);
  
  auto status = Propagator::inject(context, *trace_context_);
  ASSERT_TRUE(status.ok()) << status.message();
  
  // Should use multiple headers format by default
  EXPECT_EQ(headers_->get_("x-b3-traceid"), valid_trace_id);
  EXPECT_EQ(headers_->get_("x-b3-spanid"), valid_span_id);
  EXPECT_EQ(headers_->get_("x-b3-sampled"), "1");
  EXPECT_TRUE(headers_->get_("b3").empty());
}

// Test TracingHelper for backward compatibility
TEST_F(B3PropagatorTest, TracingHelperExtraction) {
  addB3MultipleHeaders(valid_trace_id, valid_span_id, valid_parent_span_id, "1");
  
  auto context = TracingHelper::extractForTracer(*trace_context_);
  ASSERT_TRUE(context.has_value());
  
  EXPECT_TRUE(context->isValid());
  EXPECT_EQ(context->traceId().toHexString(), valid_trace_id);
  EXPECT_EQ(context->spanId().toHexString(), valid_span_id);
  EXPECT_TRUE(TracingHelper::isSampled(context->samplingState()));
}

TEST_F(B3PropagatorTest, TracingHelperSamplingConversion) {
  EXPECT_FALSE(TracingHelper::isSampled(SamplingState::NOT_SAMPLED));
  EXPECT_TRUE(TracingHelper::isSampled(SamplingState::SAMPLED));
  EXPECT_TRUE(TracingHelper::isSampled(SamplingState::DEBUG)); // Debug implies sampled
}

TEST_F(B3PropagatorTest, TracingHelperCreateTraceContext) {
  auto context = TracingHelper::createTraceContext(
      0x1234567890abcdef, 0xfedcba0987654321, // 128-bit trace ID
      0x1111222233334444, // span ID
      0x5555666677778888, // parent span ID
      true // sampled
  );
  
  EXPECT_TRUE(context.isValid());
  EXPECT_TRUE(context.traceId().is128Bit());
  EXPECT_EQ(context.traceId().high(), 0x1234567890abcdef);
  EXPECT_EQ(context.traceId().low(), 0xfedcba0987654321);
  EXPECT_EQ(context.spanId().value(), 0x1111222233334444);
  EXPECT_TRUE(context.hasParentSpanId());
  EXPECT_EQ(context.parentSpanId().value(), 0x5555666677778888);
  EXPECT_EQ(context.samplingState(), SamplingState::SAMPLED);
}

} // namespace
} // namespace B3
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
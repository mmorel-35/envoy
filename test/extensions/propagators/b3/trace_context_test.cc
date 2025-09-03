#include "source/extensions/propagators/b3/trace_context.h"

#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace B3 {
namespace {

class TraceIdTest : public ::testing::Test {};

// Test valid 64-bit trace ID
TEST_F(TraceIdTest, ValidTraceId64Bit) {
  auto result = TraceId::fromHexString("1234567890abcdef");
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& trace_id = result.value();
  EXPECT_TRUE(trace_id.isValid());
  EXPECT_FALSE(trace_id.is128Bit());
  EXPECT_EQ(trace_id.high(), 0);
  EXPECT_EQ(trace_id.low(), 0x1234567890abcdef);
  EXPECT_EQ(trace_id.toHexString(), "1234567890abcdef");
}

// Test valid 128-bit trace ID
TEST_F(TraceIdTest, ValidTraceId128Bit) {
  auto result = TraceId::fromHexString("1234567890abcdef1234567890abcdef");
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& trace_id = result.value();
  EXPECT_TRUE(trace_id.isValid());
  EXPECT_TRUE(trace_id.is128Bit());
  EXPECT_EQ(trace_id.high(), 0x1234567890abcdef);
  EXPECT_EQ(trace_id.low(), 0x1234567890abcdef);
  EXPECT_EQ(trace_id.toHexString(), "1234567890abcdef1234567890abcdef");
}

// Test zero trace ID rejection (B3 specification requirement)
TEST_F(TraceIdTest, RejectsZeroTraceId64Bit) {
  auto result = TraceId::fromHexString("0000000000000000");
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(result.status().message(), testing::HasSubstr("zero"));
}

// Test zero trace ID rejection for 128-bit
TEST_F(TraceIdTest, RejectsZeroTraceId128Bit) {
  auto result = TraceId::fromHexString("00000000000000000000000000000000");
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(result.status().message(), testing::HasSubstr("zero"));
}

// Test invalid length rejection
TEST_F(TraceIdTest, RejectsInvalidLength) {
  // Too short
  auto result1 = TraceId::fromHexString("123456789");
  EXPECT_FALSE(result1.ok());
  EXPECT_THAT(result1.status().message(), testing::HasSubstr("length"));
  
  // Too long
  auto result2 = TraceId::fromHexString("1234567890abcdef1234567890abcdef1");
  EXPECT_FALSE(result2.ok());
  EXPECT_THAT(result2.status().message(), testing::HasSubstr("length"));
  
  // Invalid intermediate length
  auto result3 = TraceId::fromHexString("1234567890abcdef123");
  EXPECT_FALSE(result3.ok());
  EXPECT_THAT(result3.status().message(), testing::HasSubstr("length"));
}

// Test invalid hex characters
TEST_F(TraceIdTest, RejectsInvalidHexCharacters) {
  auto result1 = TraceId::fromHexString("123456789012345g");
  EXPECT_FALSE(result1.ok());
  EXPECT_THAT(result1.status().message(), testing::HasSubstr("hex"));
  
  auto result2 = TraceId::fromHexString("123456789012345-");
  EXPECT_FALSE(result2.ok());
  EXPECT_THAT(result2.status().message(), testing::HasSubstr("hex"));
}

// Test empty string rejection
TEST_F(TraceIdTest, RejectsEmptyString) {
  auto result = TraceId::fromHexString("");
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("empty"));
}

// Test case insensitivity (B3 specification allows both cases)
TEST_F(TraceIdTest, HandlesUppercaseHex) {
  auto result = TraceId::fromHexString("1234567890ABCDEF");
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& trace_id = result.value();
  EXPECT_TRUE(trace_id.isValid());
  EXPECT_EQ(trace_id.low(), 0x1234567890abcdef);
  // Output should be lowercase per convention
  EXPECT_EQ(trace_id.toHexString(), "1234567890abcdef");
}

// Test factory methods
TEST_F(TraceIdTest, From64BitFactory) {
  auto trace_id = TraceId::from64Bit(0x1234567890abcdef);
  EXPECT_TRUE(trace_id.isValid());
  EXPECT_FALSE(trace_id.is128Bit());
  EXPECT_EQ(trace_id.high(), 0);
  EXPECT_EQ(trace_id.low(), 0x1234567890abcdef);
}

TEST_F(TraceIdTest, From128BitFactory) {
  auto trace_id = TraceId::from128Bit(0x1234567890abcdef, 0xfedcba0987654321);
  EXPECT_TRUE(trace_id.isValid());
  EXPECT_TRUE(trace_id.is128Bit());
  EXPECT_EQ(trace_id.high(), 0x1234567890abcdef);
  EXPECT_EQ(trace_id.low(), 0xfedcba0987654321);
}

// Test equality operator
TEST_F(TraceIdTest, EqualityOperator) {
  auto trace_id1 = TraceId::from64Bit(0x1234567890abcdef);
  auto trace_id2 = TraceId::from64Bit(0x1234567890abcdef);
  auto trace_id3 = TraceId::from64Bit(0xfedcba0987654321);
  
  EXPECT_EQ(trace_id1, trace_id2);
  EXPECT_NE(trace_id1, trace_id3);
}

class SpanIdTest : public ::testing::Test {};

// Test valid span ID
TEST_F(SpanIdTest, ValidSpanId) {
  auto result = SpanId::fromHexString("1234567890abcdef");
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& span_id = result.value();
  EXPECT_TRUE(span_id.isValid());
  EXPECT_EQ(span_id.value(), 0x1234567890abcdef);
  EXPECT_EQ(span_id.toHexString(), "1234567890abcdef");
}

// Test zero span ID rejection (B3 specification requirement)
TEST_F(SpanIdTest, RejectsZeroSpanId) {
  auto result = SpanId::fromHexString("0000000000000000");
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(result.status().message(), testing::HasSubstr("zero"));
}

// Test invalid length rejection
TEST_F(SpanIdTest, RejectsInvalidLength) {
  // Too short
  auto result1 = SpanId::fromHexString("123456789");
  EXPECT_FALSE(result1.ok());
  EXPECT_THAT(result1.status().message(), testing::HasSubstr("length"));
  
  // Too long
  auto result2 = SpanId::fromHexString("1234567890abcdef1");
  EXPECT_FALSE(result2.ok());
  EXPECT_THAT(result2.status().message(), testing::HasSubstr("length"));
}

// Test invalid hex characters
TEST_F(SpanIdTest, RejectsInvalidHexCharacters) {
  auto result = SpanId::fromHexString("123456789012345g");
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("hex"));
}

// Test case insensitivity
TEST_F(SpanIdTest, HandlesUppercaseHex) {
  auto result = SpanId::fromHexString("1234567890ABCDEF");
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& span_id = result.value();
  EXPECT_TRUE(span_id.isValid());
  EXPECT_EQ(span_id.value(), 0x1234567890abcdef);
  // Output should be lowercase per convention
  EXPECT_EQ(span_id.toHexString(), "1234567890abcdef");
}

// Test factory method
TEST_F(SpanIdTest, From64BitFactory) {
  auto span_id = SpanId::from64Bit(0x1234567890abcdef);
  EXPECT_TRUE(span_id.isValid());
  EXPECT_EQ(span_id.value(), 0x1234567890abcdef);
}

// Test equality operator
TEST_F(SpanIdTest, EqualityOperator) {
  auto span_id1 = SpanId::from64Bit(0x1234567890abcdef);
  auto span_id2 = SpanId::from64Bit(0x1234567890abcdef);
  auto span_id3 = SpanId::from64Bit(0xfedcba0987654321);
  
  EXPECT_EQ(span_id1, span_id2);
  EXPECT_NE(span_id1, span_id3);
}

class SamplingStateTest : public ::testing::Test {};

// Test sampling state values per B3 specification
TEST_F(SamplingStateTest, StringToSamplingState) {
  EXPECT_EQ(samplingStateFromString("0"), SamplingState::NOT_SAMPLED);
  EXPECT_EQ(samplingStateFromString("1"), SamplingState::SAMPLED);
  EXPECT_EQ(samplingStateFromString("d"), SamplingState::DEBUG);
  
  // Legacy support
  EXPECT_EQ(samplingStateFromString("true"), SamplingState::SAMPLED);
  EXPECT_EQ(samplingStateFromString("false"), SamplingState::NOT_SAMPLED);
  
  // Case insensitive
  EXPECT_EQ(samplingStateFromString("D"), SamplingState::DEBUG);
  EXPECT_EQ(samplingStateFromString("TRUE"), SamplingState::SAMPLED);
  EXPECT_EQ(samplingStateFromString("FALSE"), SamplingState::NOT_SAMPLED);
  
  // Unknown values
  EXPECT_EQ(samplingStateFromString("unknown"), SamplingState::NOT_SAMPLED);
  EXPECT_EQ(samplingStateFromString(""), SamplingState::NOT_SAMPLED);
}

TEST_F(SamplingStateTest, SamplingStateToString) {
  EXPECT_EQ(samplingStateToString(SamplingState::NOT_SAMPLED), "0");
  EXPECT_EQ(samplingStateToString(SamplingState::SAMPLED), "1");
  EXPECT_EQ(samplingStateToString(SamplingState::DEBUG), "d");
}

class TraceContextTest : public ::testing::Test {};

// Test valid trace context creation
TEST_F(TraceContextTest, ValidTraceContext) {
  auto trace_id = TraceId::from64Bit(0x1234567890abcdef);
  auto span_id = SpanId::from64Bit(0xfedcba0987654321);
  auto parent_span_id = SpanId::from64Bit(0x1111222233334444);
  
  TraceContext context(trace_id, span_id);
  EXPECT_TRUE(context.isValid());
  EXPECT_EQ(context.traceId(), trace_id);
  EXPECT_EQ(context.spanId(), span_id);
  EXPECT_FALSE(context.hasParentSpanId());
  EXPECT_EQ(context.samplingState(), SamplingState::NOT_SAMPLED);
  
  TraceContext context_with_parent(trace_id, span_id, parent_span_id, SamplingState::SAMPLED);
  EXPECT_TRUE(context_with_parent.isValid());
  EXPECT_TRUE(context_with_parent.hasParentSpanId());
  EXPECT_EQ(context_with_parent.parentSpanId(), parent_span_id);
  EXPECT_EQ(context_with_parent.samplingState(), SamplingState::SAMPLED);
}

// Test invalid trace context rejection
TEST_F(TraceContextTest, InvalidTraceContext) {
  auto invalid_trace_id = TraceId(); // Default constructor creates invalid
  auto valid_span_id = SpanId::from64Bit(0xfedcba0987654321);
  auto valid_trace_id = TraceId::from64Bit(0x1234567890abcdef);
  auto invalid_span_id = SpanId(); // Default constructor creates invalid
  
  TraceContext context1(invalid_trace_id, valid_span_id);
  EXPECT_FALSE(context1.isValid());
  
  TraceContext context2(valid_trace_id, invalid_span_id);
  EXPECT_FALSE(context2.isValid());
  
  TraceContext context3(invalid_trace_id, invalid_span_id);
  EXPECT_FALSE(context3.isValid());
}

// Test sampling helper methods
TEST_F(TraceContextTest, SamplingHelpers) {
  auto trace_id = TraceId::from64Bit(0x1234567890abcdef);
  auto span_id = SpanId::from64Bit(0xfedcba0987654321);
  
  TraceContext not_sampled(trace_id, span_id, SpanId(), SamplingState::NOT_SAMPLED);
  EXPECT_FALSE(not_sampled.isSampled());
  EXPECT_FALSE(not_sampled.isDebug());
  
  TraceContext sampled(trace_id, span_id, SpanId(), SamplingState::SAMPLED);
  EXPECT_TRUE(sampled.isSampled());
  EXPECT_FALSE(sampled.isDebug());
  
  TraceContext debug(trace_id, span_id, SpanId(), SamplingState::DEBUG);
  EXPECT_TRUE(debug.isSampled()); // Debug implies sampled
  EXPECT_TRUE(debug.isDebug());
}

} // namespace
} // namespace B3
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
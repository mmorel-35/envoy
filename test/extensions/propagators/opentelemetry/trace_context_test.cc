#include "source/extensions/propagators/opentelemetry/trace_context.h"

#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace OpenTelemetry {
namespace {

class CompositeTraceContextTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Set up test W3C context
    auto w3c_traceparent = W3C::TraceParent("00", "1234567890abcdef1234567890abcdef", 
                                           "fedcba0987654321", "01");
    auto w3c_tracestate = W3C::TraceState();
    w3c_tracestate.add("vendor", "value");
    w3c_context_ = W3C::TraceContext(w3c_traceparent, w3c_tracestate);
    
    // Set up test B3 context
    auto b3_trace_id = B3::TraceId::fromHexString("1234567890abcdef1234567890abcdef").value();
    auto b3_span_id = B3::SpanId::fromHexString("fedcba0987654321").value();
    auto b3_parent_span_id = B3::SpanId::fromHexString("1111222233334444").value();
    b3_context_ = B3::TraceContext(b3_trace_id, b3_span_id, b3_parent_span_id, 
                                  B3::SamplingState::SAMPLED);
  }

  W3C::TraceContext w3c_context_;
  B3::TraceContext b3_context_;
};

// Test W3C format creation
TEST_F(CompositeTraceContextTest, CreateFromW3C) {
  CompositeTraceContext composite(w3c_context_);
  
  EXPECT_EQ(composite.format(), TraceFormat::W3C);
  EXPECT_TRUE(composite.hasW3CContext());
  EXPECT_FALSE(composite.hasB3Context());
  
  const auto& w3c = composite.w3cContext();
  EXPECT_EQ(w3c.traceParent().traceId(), "1234567890abcdef1234567890abcdef");
  EXPECT_EQ(w3c.traceParent().parentId(), "fedcba0987654321");
  EXPECT_TRUE(w3c.traceParent().isSampled());
  EXPECT_TRUE(w3c.hasTraceState());
}

// Test B3 format creation
TEST_F(CompositeTraceContextTest, CreateFromB3) {
  CompositeTraceContext composite(b3_context_);
  
  EXPECT_EQ(composite.format(), TraceFormat::B3);
  EXPECT_FALSE(composite.hasW3CContext());
  EXPECT_TRUE(composite.hasB3Context());
  
  const auto& b3 = composite.b3Context();
  EXPECT_EQ(b3.traceId().toHexString(), "1234567890abcdef1234567890abcdef");
  EXPECT_EQ(b3.spanId().toHexString(), "fedcba0987654321");
  EXPECT_TRUE(b3.hasParentSpanId());
  EXPECT_TRUE(b3.isSampled());
}

// Test composite context with both formats
TEST_F(CompositeTraceContextTest, CreateWithBothFormats) {
  CompositeTraceContext composite(w3c_context_, b3_context_);
  
  EXPECT_EQ(composite.format(), TraceFormat::W3C); // Primary format
  EXPECT_TRUE(composite.hasW3CContext());
  EXPECT_TRUE(composite.hasB3Context());
  
  // Both contexts should be accessible
  EXPECT_EQ(composite.w3cContext().traceParent().traceId(), 
           composite.b3Context().traceId().toHexString());
  EXPECT_EQ(composite.w3cContext().traceParent().parentId(),
           composite.b3Context().spanId().toHexString());
}

// Test trace ID extraction
TEST_F(CompositeTraceContextTest, GetTraceId) {
  CompositeTraceContext w3c_composite(w3c_context_);
  CompositeTraceContext b3_composite(b3_context_);
  
  EXPECT_EQ(w3c_composite.getTraceId(), "1234567890abcdef1234567890abcdef");
  EXPECT_EQ(b3_composite.getTraceId(), "1234567890abcdef1234567890abcdef");
}

// Test span ID extraction
TEST_F(CompositeTraceContextTest, GetSpanId) {
  CompositeTraceContext w3c_composite(w3c_context_);
  CompositeTraceContext b3_composite(b3_context_);
  
  EXPECT_EQ(w3c_composite.getSpanId(), "fedcba0987654321");
  EXPECT_EQ(b3_composite.getSpanId(), "fedcba0987654321");
}

// Test parent span ID extraction
TEST_F(CompositeTraceContextTest, GetParentSpanId) {
  CompositeTraceContext w3c_composite(w3c_context_);
  CompositeTraceContext b3_composite(b3_context_);
  
  // W3C uses parentId from traceparent, B3 uses separate parent span ID
  EXPECT_FALSE(w3c_composite.getParentSpanId().empty());
  EXPECT_FALSE(b3_composite.getParentSpanId().empty());
}

// Test sampling state
TEST_F(CompositeTraceContextTest, IsSampled) {
  CompositeTraceContext w3c_composite(w3c_context_);
  CompositeTraceContext b3_composite(b3_context_);
  
  EXPECT_TRUE(w3c_composite.isSampled());
  EXPECT_TRUE(b3_composite.isSampled());
}

// Test format conversion
TEST_F(CompositeTraceContextTest, ConvertToW3C) {
  CompositeTraceContext b3_composite(b3_context_);
  
  auto w3c_result = b3_composite.toW3C();
  ASSERT_TRUE(w3c_result.ok()) << w3c_result.status().message();
  
  const auto& converted = w3c_result.value();
  EXPECT_EQ(converted.traceParent().traceId(), b3_context_.traceId().toHexString());
  EXPECT_EQ(converted.traceParent().parentId(), b3_context_.spanId().toHexString());
  EXPECT_EQ(converted.traceParent().isSampled(), b3_context_.isSampled());
}

TEST_F(CompositeTraceContextTest, ConvertToB3) {
  CompositeTraceContext w3c_composite(w3c_context_);
  
  auto b3_result = w3c_composite.toB3();
  ASSERT_TRUE(b3_result.ok()) << b3_result.status().message();
  
  const auto& converted = b3_result.value();
  EXPECT_EQ(converted.traceId().toHexString(), w3c_context_.traceParent().traceId());
  EXPECT_EQ(converted.spanId().toHexString(), w3c_context_.traceParent().parentId());
  EXPECT_EQ(converted.isSampled(), w3c_context_.traceParent().isSampled());
}

// Test child span creation
TEST_F(CompositeTraceContextTest, CreateChild) {
  CompositeTraceContext parent(w3c_context_);
  std::string new_span_id = "aaaaaaaaaaaaaaaa";
  
  auto child_result = parent.createChild(new_span_id);
  ASSERT_TRUE(child_result.ok()) << child_result.status().message();
  
  const auto& child = child_result.value();
  EXPECT_EQ(child.getTraceId(), parent.getTraceId()); // Same trace ID
  EXPECT_EQ(child.getSpanId(), new_span_id); // New span ID
  EXPECT_EQ(child.getParentSpanId(), parent.getSpanId()); // Parent span ID set
  EXPECT_EQ(child.isSampled(), parent.isSampled()); // Inherit sampling
}

// Test equality operator
TEST_F(CompositeTraceContextTest, EqualityOperator) {
  CompositeTraceContext composite1(w3c_context_);
  CompositeTraceContext composite2(w3c_context_);
  CompositeTraceContext composite3(b3_context_);
  
  EXPECT_EQ(composite1, composite2);
  EXPECT_NE(composite1, composite3);
}

class CompositeBaggageTest : public ::testing::Test {
protected:
  void SetUp() override {
    W3C::Baggage w3c_baggage;
    w3c_baggage.set(W3C::BaggageMember("key1", "value1"));
    w3c_baggage.set(W3C::BaggageMember("key2", "value2"));
    baggage_ = CompositeBaggage(w3c_baggage);
  }

  CompositeBaggage baggage_;
};

// Test baggage creation and access
TEST_F(CompositeBaggageTest, CreateAndAccess) {
  EXPECT_TRUE(baggage_.hasValues());
  EXPECT_EQ(baggage_.getValue("key1"), "value1");
  EXPECT_EQ(baggage_.getValue("key2"), "value2");
  EXPECT_TRUE(baggage_.getValue("nonexistent").empty());
}

// Test baggage modification
TEST_F(CompositeBaggageTest, SetAndRemove) {
  baggage_.setValue("key3", "value3");
  EXPECT_EQ(baggage_.getValue("key3"), "value3");
  
  baggage_.removeValue("key1");
  EXPECT_TRUE(baggage_.getValue("key1").empty());
}

// Test baggage conversion
TEST_F(CompositeBaggageTest, ToW3C) {
  auto w3c_result = baggage_.toW3C();
  ASSERT_TRUE(w3c_result.ok()) << w3c_result.status().message();
  
  const auto& w3c_baggage = w3c_result.value();
  EXPECT_EQ(w3c_baggage.get("key1"), "value1");
  EXPECT_EQ(w3c_baggage.get("key2"), "value2");
}

// Test baggage size limits
TEST_F(CompositeBaggageTest, SizeLimits) {
  // Test that large values are rejected
  std::string large_value(5000, 'a'); // Larger than typical limit
  bool result = baggage_.setValue("large_key", large_value);
  EXPECT_FALSE(result); // Should be rejected due to size
}

} // namespace
} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
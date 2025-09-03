#include "source/extensions/propagators/w3c/propagator.h"

#include "source/common/http/header_map_impl.h"
#include "source/common/tracing/trace_context_impl.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3C {
namespace {

class PropagatorTest : public ::testing::Test {
protected:
  void SetUp() override {
    headers_ = Http::RequestHeaderMapImpl::create();
    trace_context_ = std::make_unique<Tracing::TraceContextImpl>(*headers_);
  }

  Http::RequestHeaderMapPtr headers_;
  std::unique_ptr<Tracing::TraceContextImpl> trace_context_;
  
  const std::string valid_traceparent = "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01";
  const std::string valid_tracestate = "congo=t61rcWkgMzE,rojo=00f067aa0ba902b7";
};

TEST_F(PropagatorTest, IsNotPresentWhenEmpty) {
  EXPECT_FALSE(Propagator::isPresent(*trace_context_));
}

TEST_F(PropagatorTest, IsPresentWithTraceparent) {
  headers_->addCopy("traceparent", valid_traceparent);
  EXPECT_TRUE(Propagator::isPresent(*trace_context_));
}

TEST_F(PropagatorTest, ExtractWithTraceparentOnly) {
  headers_->addCopy("traceparent", valid_traceparent);
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_EQ(context.traceParent().toString(), valid_traceparent);
  EXPECT_FALSE(context.hasTraceState());
}

TEST_F(PropagatorTest, ExtractWithBothHeaders) {
  headers_->addCopy("traceparent", valid_traceparent);
  headers_->addCopy("tracestate", valid_tracestate);
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_EQ(context.traceParent().toString(), valid_traceparent);
  EXPECT_TRUE(context.hasTraceState());
  EXPECT_EQ(context.traceState().toString(), valid_tracestate);
}

TEST_F(PropagatorTest, ExtractWithMultipleTracestateHeaders) {
  headers_->addCopy("traceparent", valid_traceparent);
  headers_->addCopy("tracestate", "congo=t61rcWkgMzE");
  headers_->addCopy("tracestate", "rojo=00f067aa0ba902b7");
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  EXPECT_TRUE(context.hasTraceState());
  
  // Should have both entries (combined)
  auto congo_value = context.traceState().get("congo");
  ASSERT_TRUE(congo_value.has_value());
  EXPECT_EQ(congo_value.value(), "t61rcWkgMzE");
  
  auto rojo_value = context.traceState().get("rojo");
  ASSERT_TRUE(rojo_value.has_value());
  EXPECT_EQ(rojo_value.value(), "00f067aa0ba902b7");
}

TEST_F(PropagatorTest, ExtractFailsWithoutTraceparent) {
  headers_->addCopy("tracestate", valid_tracestate);
  
  auto result = Propagator::extract(*trace_context_);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST_F(PropagatorTest, ExtractFailsWithInvalidTraceparent) {
  headers_->addCopy("traceparent", "invalid-traceparent");
  
  auto result = Propagator::extract(*trace_context_);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST_F(PropagatorTest, InjectTraceparentOnly) {
  auto traceparent = TraceParent::parse(valid_traceparent).value();
  TraceContext context(traceparent);
  
  Propagator::inject(context, *trace_context_);
  
  auto traceparent_header = headers_->get(Http::LowerCaseString("traceparent"));
  ASSERT_FALSE(traceparent_header.empty());
  EXPECT_EQ(traceparent_header[0]->value().getStringView(), valid_traceparent);
  
  auto tracestate_header = headers_->get(Http::LowerCaseString("tracestate"));
  EXPECT_TRUE(tracestate_header.empty());
}

TEST_F(PropagatorTest, InjectBothHeaders) {
  auto traceparent = TraceParent::parse(valid_traceparent).value();
  auto tracestate = TraceState::parse(valid_tracestate).value();
  TraceContext context(traceparent, tracestate);
  
  Propagator::inject(context, *trace_context_);
  
  auto traceparent_header = headers_->get(Http::LowerCaseString("traceparent"));
  ASSERT_FALSE(traceparent_header.empty());
  EXPECT_EQ(traceparent_header[0]->value().getStringView(), valid_traceparent);
  
  auto tracestate_header = headers_->get(Http::LowerCaseString("tracestate"));
  ASSERT_FALSE(tracestate_header.empty());
  EXPECT_EQ(tracestate_header[0]->value().getStringView(), valid_tracestate);
}

TEST_F(PropagatorTest, CreateChild) {
  auto traceparent = TraceParent::parse(valid_traceparent).value();
  auto tracestate = TraceState::parse(valid_tracestate).value();
  TraceContext parent_context(traceparent, tracestate);
  
  std::string new_span_id = "b7ad6b7169203331";
  auto result = Propagator::createChild(parent_context, new_span_id);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& child_context = result.value();
  
  // Should have same trace ID but new span ID
  EXPECT_EQ(child_context.traceParent().traceId(), traceparent.traceId());
  EXPECT_EQ(child_context.traceParent().parentId(), new_span_id);
  EXPECT_EQ(child_context.traceParent().version(), traceparent.version());
  EXPECT_EQ(child_context.traceParent().traceFlags(), traceparent.traceFlags());
  
  // Should inherit tracestate
  EXPECT_TRUE(child_context.hasTraceState());
  EXPECT_EQ(child_context.traceState().toString(), tracestate.toString());
}

TEST_F(PropagatorTest, CreateChildInvalidSpanId) {
  auto traceparent = TraceParent::parse(valid_traceparent).value();
  TraceContext parent_context(traceparent);
  
  // Invalid span ID (wrong length)
  auto result = Propagator::createChild(parent_context, "invalid");
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST_F(PropagatorTest, CreateRoot) {
  std::string trace_id = "4bf92f3577b34da6a3ce929d0e0e4736";
  std::string span_id = "00f067aa0ba902b7";
  
  auto result = Propagator::createRoot(trace_id, span_id, true);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& root_context = result.value();
  
  EXPECT_EQ(root_context.traceParent().version(), "00");
  EXPECT_EQ(root_context.traceParent().traceId(), trace_id);
  EXPECT_EQ(root_context.traceParent().parentId(), span_id);
  EXPECT_EQ(root_context.traceParent().traceFlags(), "01");
  EXPECT_TRUE(root_context.traceParent().isSampled());
  EXPECT_FALSE(root_context.hasTraceState());
}

TEST_F(PropagatorTest, CreateRootNotSampled) {
  std::string trace_id = "4bf92f3577b34da6a3ce929d0e0e4736";
  std::string span_id = "00f067aa0ba902b7";
  
  auto result = Propagator::createRoot(trace_id, span_id, false);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& root_context = result.value();
  
  EXPECT_EQ(root_context.traceParent().traceFlags(), "00");
  EXPECT_FALSE(root_context.traceParent().isSampled());
}

TEST_F(PropagatorTest, CreateRootInvalidInputs) {
  // Invalid trace ID
  auto result1 = Propagator::createRoot("invalid", "00f067aa0ba902b7", true);
  EXPECT_FALSE(result1.ok());
  
  // Invalid span ID
  auto result2 = Propagator::createRoot("4bf92f3577b34da6a3ce929d0e0e4736", "invalid", true);
  EXPECT_FALSE(result2.ok());
}

class TracingHelperTest : public ::testing::Test {
protected:
  void SetUp() override {
    headers_ = Http::RequestHeaderMapImpl::create();
    trace_context_ = std::make_unique<Tracing::TraceContextImpl>(*headers_);
  }

  Http::RequestHeaderMapPtr headers_;
  std::unique_ptr<Tracing::TraceContextImpl> trace_context_;
  
  const std::string valid_traceparent = "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01";
  const std::string valid_tracestate = "congo=t61rcWkgMzE,rojo=00f067aa0ba902b7";
};

TEST_F(TracingHelperTest, ExtractForTracer) {
  headers_->addCopy("traceparent", valid_traceparent);
  headers_->addCopy("tracestate", valid_tracestate);
  
  auto result = TracingHelper::extractForTracer(*trace_context_);
  ASSERT_TRUE(result.has_value());
  
  const auto& extracted = result.value();
  EXPECT_EQ(extracted.version, "00");
  EXPECT_EQ(extracted.trace_id, "4bf92f3577b34da6a3ce929d0e0e4736");
  EXPECT_EQ(extracted.span_id, "00f067aa0ba902b7");
  EXPECT_EQ(extracted.trace_flags, "01");
  EXPECT_TRUE(extracted.sampled);
  EXPECT_EQ(extracted.tracestate, valid_tracestate);
}

TEST_F(TracingHelperTest, ExtractForTracerNoHeaders) {
  auto result = TracingHelper::extractForTracer(*trace_context_);
  EXPECT_FALSE(result.has_value());
}

TEST_F(TracingHelperTest, TraceparentPresent) {
  EXPECT_FALSE(TracingHelper::traceparentPresent(*trace_context_));
  
  headers_->addCopy("traceparent", valid_traceparent);
  EXPECT_TRUE(TracingHelper::traceparentPresent(*trace_context_));
}

TEST_F(TracingHelperTest, RoundTripThroughPropagator) {
  // Inject a context
  auto traceparent = TraceParent::parse(valid_traceparent).value();
  auto tracestate = TraceState::parse(valid_tracestate).value();
  TraceContext original_context(traceparent, tracestate);
  
  Propagator::inject(original_context, *trace_context_);
  
  // Extract it back
  auto extracted_result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(extracted_result.ok());
  
  const auto& extracted_context = extracted_result.value();
  EXPECT_EQ(extracted_context.traceParent().toString(), original_context.traceParent().toString());
  EXPECT_EQ(extracted_context.traceState().toString(), original_context.traceState().toString());
}

// Baggage Tests
class BaggagePropagatorTest : public ::testing::Test {
protected:
  void SetUp() override {
    headers_ = Http::RequestHeaderMapImpl::create();
    trace_context_ = std::make_unique<Tracing::TraceContextImpl>(*headers_);
  }

  Http::RequestHeaderMapPtr headers_;
  std::unique_ptr<Tracing::TraceContextImpl> trace_context_;
  
  const std::string valid_baggage = "key1=value1,key2=value2;prop1=propvalue1";
  const std::string simple_baggage = "userId=alice,sessionId=xyz";
};

TEST_F(BaggagePropagatorTest, IsBaggageNotPresentWhenEmpty) {
  EXPECT_FALSE(Propagator::isBaggagePresent(*trace_context_));
}

TEST_F(BaggagePropagatorTest, IsBaggagePresentWhenHeaderExists) {
  headers_->addCopy(Http::LowerCaseString("baggage"), valid_baggage);
  EXPECT_TRUE(Propagator::isBaggagePresent(*trace_context_));
}

TEST_F(BaggagePropagatorTest, ExtractValidBaggage) {
  headers_->addCopy(Http::LowerCaseString("baggage"), valid_baggage);
  
  auto result = Propagator::extractBaggage(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& baggage = result.value();
  EXPECT_EQ(baggage.getMembers().size(), 2);
  
  auto value1 = baggage.get("key1");
  ASSERT_TRUE(value1.has_value());
  EXPECT_EQ(value1.value(), "value1");
  
  auto value2 = baggage.get("key2");
  ASSERT_TRUE(value2.has_value());
  EXPECT_EQ(value2.value(), "value2");
}

TEST_F(BaggagePropagatorTest, ExtractEmptyWhenNoBaggageHeader) {
  auto result = Propagator::extractBaggage(*trace_context_);
  ASSERT_TRUE(result.ok());
  EXPECT_TRUE(result.value().empty());
}

TEST_F(BaggagePropagatorTest, InjectBaggage) {
  auto baggage_result = Baggage::parse(simple_baggage);
  ASSERT_TRUE(baggage_result.ok());
  
  Propagator::injectBaggage(baggage_result.value(), *trace_context_);
  
  // Check that the header was set
  EXPECT_TRUE(Propagator::isBaggagePresent(*trace_context_));
  
  // Extract and verify
  auto extracted = Propagator::extractBaggage(*trace_context_);
  ASSERT_TRUE(extracted.ok());
  
  auto userId = extracted.value().get("userId");
  ASSERT_TRUE(userId.has_value());
  EXPECT_EQ(userId.value(), "alice");
  
  auto sessionId = extracted.value().get("sessionId");
  ASSERT_TRUE(sessionId.has_value());
  EXPECT_EQ(sessionId.value(), "xyz");
}

TEST_F(BaggagePropagatorTest, BaggageRoundTrip) {
  // Create original baggage
  auto original_baggage = Baggage::parse(valid_baggage);
  ASSERT_TRUE(original_baggage.ok());
  
  // Inject it
  Propagator::injectBaggage(original_baggage.value(), *trace_context_);
  
  // Extract it back
  auto extracted_baggage = Propagator::extractBaggage(*trace_context_);
  ASSERT_TRUE(extracted_baggage.ok());
  
  // Compare
  EXPECT_EQ(original_baggage.value().getMembers().size(), extracted_baggage.value().getMembers().size());
  
  for (const auto& member : original_baggage.value().getMembers()) {
    auto value = extracted_baggage.value().get(member.key());
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), member.value());
  }
}

// BaggageHelper Tests
class BaggageHelperTest : public ::testing::Test {
protected:
  void SetUp() override {
    headers_ = Http::RequestHeaderMapImpl::create();
    trace_context_ = std::make_unique<Tracing::TraceContextImpl>(*headers_);
  }

  Http::RequestHeaderMapPtr headers_;
  std::unique_ptr<Tracing::TraceContextImpl> trace_context_;
};

TEST_F(BaggageHelperTest, GetBaggageValueWhenPresent) {
  headers_->addCopy(Http::LowerCaseString("baggage"), "key1=value1,key2=value2");
  
  std::string value1 = BaggageHelper::getBaggageValue(*trace_context_, "key1");
  EXPECT_EQ(value1, "value1");
  
  std::string value2 = BaggageHelper::getBaggageValue(*trace_context_, "key2");
  EXPECT_EQ(value2, "value2");
}

TEST_F(BaggageHelperTest, GetBaggageValueWhenNotPresent) {
  std::string value = BaggageHelper::getBaggageValue(*trace_context_, "nonexistent");
  EXPECT_EQ(value, "");
}

TEST_F(BaggageHelperTest, SetBaggageValue) {
  EXPECT_TRUE(BaggageHelper::setBaggageValue(*trace_context_, "testKey", "testValue"));
  
  std::string retrieved = BaggageHelper::getBaggageValue(*trace_context_, "testKey");
  EXPECT_EQ(retrieved, "testValue");
}

TEST_F(BaggageHelperTest, GetAllBaggage) {
  headers_->addCopy(Http::LowerCaseString("baggage"), "key1=value1,key2=value2,key3=value3");
  
  auto all_baggage = BaggageHelper::getAllBaggage(*trace_context_);
  EXPECT_EQ(all_baggage.size(), 3);
  EXPECT_EQ(all_baggage["key1"], "value1");
  EXPECT_EQ(all_baggage["key2"], "value2");
  EXPECT_EQ(all_baggage["key3"], "value3");
}

TEST_F(BaggageHelperTest, HasBaggage) {
  EXPECT_FALSE(BaggageHelper::hasBaggage(*trace_context_));
  
  headers_->addCopy(Http::LowerCaseString("baggage"), "key=value");
  EXPECT_TRUE(BaggageHelper::hasBaggage(*trace_context_));
}

// Integration test: Complete W3C context with baggage
TEST_F(PropagatorTest, ExtractCompleteW3CContextWithBaggage) {
  // Set all W3C headers
  headers_->addCopy(Http::LowerCaseString("traceparent"), valid_traceparent);
  headers_->addCopy(Http::LowerCaseString("tracestate"), valid_tracestate);
  headers_->addCopy(Http::LowerCaseString("baggage"), "userId=alice,sessionId=xyz123");
  
  auto result = Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok()) << result.status().message();
  
  const auto& context = result.value();
  
  // Check traceparent
  EXPECT_EQ(context.traceParent().toString(), valid_traceparent);
  
  // Check tracestate
  EXPECT_TRUE(context.hasTraceState());
  auto congo_value = context.traceState().get("congo");
  ASSERT_TRUE(congo_value.has_value());
  EXPECT_EQ(congo_value.value(), "t61rcWkgMzE");
  
  // Check baggage
  EXPECT_TRUE(context.hasBaggage());
  auto user_id = context.baggage().get("userId");
  ASSERT_TRUE(user_id.has_value());
  EXPECT_EQ(user_id.value(), "alice");
  
  auto session_id = context.baggage().get("sessionId");
  ASSERT_TRUE(session_id.has_value());
  EXPECT_EQ(session_id.value(), "xyz123");
}

TEST_F(PropagatorTest, InjectCompleteW3CContextWithBaggage) {
  // Create a complete W3C context
  auto traceparent = TraceParent::parse(valid_traceparent);
  ASSERT_TRUE(traceparent.ok());
  
  auto tracestate = TraceState::parse(valid_tracestate);
  ASSERT_TRUE(tracestate.ok());
  
  auto baggage = Baggage::parse("userId=bob,sessionId=abc123");
  ASSERT_TRUE(baggage.ok());
  
  TraceContext context(std::move(traceparent.value()), std::move(tracestate.value()), std::move(baggage.value()));
  
  // Inject it
  Propagator::inject(context, *trace_context_);
  
  // Verify all headers were set
  EXPECT_TRUE(Propagator::isPresent(*trace_context_));
  EXPECT_TRUE(Propagator::isBaggagePresent(*trace_context_));
  
  // Extract and verify
  auto extracted = Propagator::extract(*trace_context_);
  ASSERT_TRUE(extracted.ok());
  
  EXPECT_EQ(extracted.value().traceParent().toString(), context.traceParent().toString());
  EXPECT_TRUE(extracted.value().hasTraceState());
  EXPECT_TRUE(extracted.value().hasBaggage());
  
  auto extracted_user = extracted.value().baggage().get("userId");
  ASSERT_TRUE(extracted_user.has_value());
  EXPECT_EQ(extracted_user.value(), "bob");
}
}

} // namespace
} // namespace W3C
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
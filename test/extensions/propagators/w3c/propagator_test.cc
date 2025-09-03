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

} // namespace
} // namespace W3C
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
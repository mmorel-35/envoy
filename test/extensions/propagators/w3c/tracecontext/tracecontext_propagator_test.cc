#include "source/extensions/propagators/w3c/tracecontext/tracecontext_propagator.h"

#include "test/test_common/utility.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3c {
namespace TraceContext {
namespace {

class TraceContextPropagatorTest : public testing::Test {
protected:
  TraceContextPropagator propagator_;
  
  // Valid test values per W3C specification
  const std::string valid_version_ = "00";
  const std::string valid_trace_id_ = "4bf92f3577b34da6a3ce929d0e0e4736";
  const std::string valid_span_id_ = "00f067aa0ba902b7";
  const std::string valid_flags_sampled_ = "01";
  const std::string valid_flags_not_sampled_ = "00";
  const std::string valid_traceparent_sampled_ = "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01";
  const std::string valid_traceparent_not_sampled_ = "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-00";
  const std::string valid_tracestate_ = "rojo=00f067aa0ba902b7,congo=t61rcWkgMzE";
};

TEST_F(TraceContextPropagatorTest, ExtractTraceParentSuccess) {
  Tracing::TestTraceContextImpl trace_context{{"traceparent", valid_traceparent_sampled_}};
  
  auto result = propagator_.extractTraceParent(trace_context);
  
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), valid_traceparent_sampled_);
}

TEST_F(TraceContextPropagatorTest, ExtractTraceParentNotPresent) {
  Tracing::TestTraceContextImpl trace_context{};
  
  auto result = propagator_.extractTraceParent(trace_context);
  
  EXPECT_FALSE(result.has_value());
}

TEST_F(TraceContextPropagatorTest, ExtractTraceStateSuccess) {
  Tracing::TestTraceContextImpl trace_context{{"tracestate", valid_tracestate_}};
  
  auto result = propagator_.extractTraceState(trace_context);
  
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), valid_tracestate_);
}

TEST_F(TraceContextPropagatorTest, ExtractTraceStateMultipleValues) {
  Tracing::TestTraceContextImpl trace_context{{"tracestate", "rojo=00f067aa0ba902b7"},
                                               {"tracestate", "congo=t61rcWkgMzE"}};
  
  auto result = propagator_.extractTraceState(trace_context);
  
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), "rojo=00f067aa0ba902b7,congo=t61rcWkgMzE");
}

TEST_F(TraceContextPropagatorTest, ExtractTraceStateNotPresent) {
  Tracing::TestTraceContextImpl trace_context{};
  
  auto result = propagator_.extractTraceState(trace_context);
  
  EXPECT_FALSE(result.has_value());
}

TEST_F(TraceContextPropagatorTest, ParseTraceParentSuccess) {
  auto result = propagator_.parseTraceParent(valid_traceparent_sampled_);
  
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result->version, valid_version_);
  EXPECT_EQ(result->trace_id, valid_trace_id_);
  EXPECT_EQ(result->span_id, valid_span_id_);
  EXPECT_TRUE(result->sampled);
}

TEST_F(TraceContextPropagatorTest, ParseTraceParentNotSampled) {
  auto result = propagator_.parseTraceParent(valid_traceparent_not_sampled_);
  
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result->version, valid_version_);
  EXPECT_EQ(result->trace_id, valid_trace_id_);
  EXPECT_EQ(result->span_id, valid_span_id_);
  EXPECT_FALSE(result->sampled);
}

TEST_F(TraceContextPropagatorTest, ParseTraceParentInvalidLength) {
  // Too short
  auto result1 = propagator_.parseTraceParent("00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7");
  EXPECT_FALSE(result1.ok());
  EXPECT_THAT(result1.status().message(), testing::HasSubstr("Invalid traceparent header length"));
  
  // Too long
  auto result2 = propagator_.parseTraceParent("00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01-extra");
  EXPECT_FALSE(result2.ok());
  EXPECT_THAT(result2.status().message(), testing::HasSubstr("Invalid traceparent header length"));
}

TEST_F(TraceContextPropagatorTest, ParseTraceParentInvalidHyphenation) {
  auto result = propagator_.parseTraceParent("00_4bf92f3577b34da6a3ce929d0e0e4736_00f067aa0ba902b7_01");
  
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("Invalid traceparent hyphenation"));
}

TEST_F(TraceContextPropagatorTest, ParseTraceParentInvalidFieldSizes) {
  // Version too short
  auto result1 = propagator_.parseTraceParent("0-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01");
  EXPECT_FALSE(result1.ok());
  EXPECT_THAT(result1.status().message(), testing::HasSubstr("Invalid traceparent field sizes"));
  
  // Trace ID too short
  auto result2 = propagator_.parseTraceParent("00-4bf92f3577b34da6a3ce929d0e0e473-00f067aa0ba902b7-01");
  EXPECT_FALSE(result2.ok());
  EXPECT_THAT(result2.status().message(), testing::HasSubstr("Invalid traceparent field sizes"));
  
  // Span ID too short
  auto result3 = propagator_.parseTraceParent("00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902-01");
  EXPECT_FALSE(result3.ok());
  EXPECT_THAT(result3.status().message(), testing::HasSubstr("Invalid traceparent field sizes"));
  
  // Flags too short
  auto result4 = propagator_.parseTraceParent("00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-1");
  EXPECT_FALSE(result4.ok());
  EXPECT_THAT(result4.status().message(), testing::HasSubstr("Invalid traceparent field sizes"));
}

TEST_F(TraceContextPropagatorTest, ParseTraceParentInvalidHex) {
  // Invalid hex in trace ID
  auto result1 = propagator_.parseTraceParent("00-4bf92f3577b34da6a3ce929d0e0e473g-00f067aa0ba902b7-01");
  EXPECT_FALSE(result1.ok());
  EXPECT_THAT(result1.status().message(), testing::HasSubstr("Invalid header hex"));
  
  // Invalid hex in span ID
  auto result2 = propagator_.parseTraceParent("00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902bg-01");
  EXPECT_FALSE(result2.ok());
  EXPECT_THAT(result2.status().message(), testing::HasSubstr("Invalid header hex"));
}

TEST_F(TraceContextPropagatorTest, ParseTraceParentInvalidTraceId) {
  // All zeros trace ID is invalid
  auto result = propagator_.parseTraceParent("00-00000000000000000000000000000000-00f067aa0ba902b7-01");
  
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("Invalid trace id"));
}

TEST_F(TraceContextPropagatorTest, ParseTraceParentInvalidSpanId) {
  // All zeros span ID is invalid
  auto result = propagator_.parseTraceParent("00-4bf92f3577b34da6a3ce929d0e0e4736-0000000000000000-01");
  
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("Invalid span id"));
}

TEST_F(TraceContextPropagatorTest, InjectTraceParentSampled) {
  Tracing::TestTraceContextImpl trace_context{};
  
  propagator_.injectTraceParent(trace_context, valid_version_, valid_trace_id_, 
                               valid_span_id_, true);
  
  auto result = propagator_.extractTraceParent(trace_context);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), valid_traceparent_sampled_);
}

TEST_F(TraceContextPropagatorTest, InjectTraceParentNotSampled) {
  Tracing::TestTraceContextImpl trace_context{};
  
  propagator_.injectTraceParent(trace_context, valid_version_, valid_trace_id_, 
                               valid_span_id_, false);
  
  auto result = propagator_.extractTraceParent(trace_context);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), valid_traceparent_not_sampled_);
}

TEST_F(TraceContextPropagatorTest, InjectTraceState) {
  Tracing::TestTraceContextImpl trace_context{};
  
  propagator_.injectTraceState(trace_context, valid_tracestate_);
  
  auto result = propagator_.extractTraceState(trace_context);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), valid_tracestate_);
}

TEST_F(TraceContextPropagatorTest, InjectTraceStateEmpty) {
  Tracing::TestTraceContextImpl trace_context{};
  
  propagator_.injectTraceState(trace_context, "");
  
  auto result = propagator_.extractTraceState(trace_context);
  EXPECT_FALSE(result.has_value());
}

TEST_F(TraceContextPropagatorTest, RemoveTraceParent) {
  Tracing::TestTraceContextImpl trace_context{{"traceparent", valid_traceparent_sampled_}};
  
  EXPECT_TRUE(propagator_.hasTraceParent(trace_context));
  
  propagator_.removeTraceParent(trace_context);
  
  EXPECT_FALSE(propagator_.hasTraceParent(trace_context));
  auto result = propagator_.extractTraceParent(trace_context);
  EXPECT_FALSE(result.has_value());
}

TEST_F(TraceContextPropagatorTest, RemoveTraceState) {
  Tracing::TestTraceContextImpl trace_context{{"tracestate", valid_tracestate_}};
  
  EXPECT_TRUE(propagator_.extractTraceState(trace_context).has_value());
  
  propagator_.removeTraceState(trace_context);
  
  auto result = propagator_.extractTraceState(trace_context);
  EXPECT_FALSE(result.has_value());
}

TEST_F(TraceContextPropagatorTest, HasTraceParent) {
  Tracing::TestTraceContextImpl trace_context_with{{"traceparent", valid_traceparent_sampled_}};
  Tracing::TestTraceContextImpl trace_context_without{};
  
  EXPECT_TRUE(propagator_.hasTraceParent(trace_context_with));
  EXPECT_FALSE(propagator_.hasTraceParent(trace_context_without));
}

TEST_F(TraceContextPropagatorTest, RoundTripTraceParent) {
  Tracing::TestTraceContextImpl trace_context{};
  
  // Inject
  propagator_.injectTraceParent(trace_context, valid_version_, valid_trace_id_, 
                               valid_span_id_, true);
  
  // Extract and parse
  auto extracted = propagator_.extractTraceParent(trace_context);
  ASSERT_TRUE(extracted.has_value());
  
  auto parsed = propagator_.parseTraceParent(extracted.value());
  ASSERT_TRUE(parsed.ok());
  
  // Verify values
  EXPECT_EQ(parsed->version, valid_version_);
  EXPECT_EQ(parsed->trace_id, valid_trace_id_);
  EXPECT_EQ(parsed->span_id, valid_span_id_);
  EXPECT_TRUE(parsed->sampled);
}

TEST_F(TraceContextPropagatorTest, RoundTripTraceState) {
  Tracing::TestTraceContextImpl trace_context{};
  
  // Inject
  propagator_.injectTraceState(trace_context, valid_tracestate_);
  
  // Extract
  auto extracted = propagator_.extractTraceState(trace_context);
  ASSERT_TRUE(extracted.has_value());
  EXPECT_EQ(extracted.value(), valid_tracestate_);
}

// Edge case tests for various trace flag values
TEST_F(TraceContextPropagatorTest, TraceFlagsVariations) {
  // Test different flag values
  std::vector<std::pair<std::string, bool>> flag_tests = {
    {"00", false}, // No flags set
    {"01", true},  // Sampled flag set
    {"02", false}, // Other flag set but not sampled
    {"03", true},  // Sampled + other flag
    {"ff", true},  // All flags set (including sampled)
    {"fe", false}, // All flags except sampled
  };
  
  for (const auto& [flags, expected_sampled] : flag_tests) {
    std::string traceparent = "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-" + flags;
    auto result = propagator_.parseTraceParent(traceparent);
    
    ASSERT_TRUE(result.ok()) << "Failed for flags: " << flags;
    EXPECT_EQ(result->sampled, expected_sampled) << "Failed for flags: " << flags;
  }
}

} // namespace
} // namespace TraceContext
} // namespace W3c
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
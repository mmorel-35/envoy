#include "source/extensions/propagators/w3c/tracecontext/tracecontext_propagator.h"
#include "source/extensions/propagators/w3c/baggage/baggage_propagator.h"

#include "test/test_common/utility.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3c {
namespace {

class W3cIntegrationTest : public testing::Test {
protected:
  TraceContext::TraceContextPropagator trace_propagator_;
  Baggage::BaggagePropagator baggage_propagator_;
};

TEST_F(W3cIntegrationTest, FullPropagationWorkflow) {
  // Simulate incoming request with W3C headers
  Tracing::TestTraceContextImpl incoming_context{
    {"traceparent", "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01"},
    {"tracestate", "rojo=00f067aa0ba902b7,congo=t61rcWkgMzE"},
    {"baggage", "userId=alice,serverNode=DF%2028,isProduction=false"}
  };

  // Extract incoming trace context
  auto incoming_traceparent = trace_propagator_.extractTraceParent(incoming_context);
  ASSERT_TRUE(incoming_traceparent.has_value());
  
  auto parsed_traceparent = trace_propagator_.parseTraceParent(incoming_traceparent.value());
  ASSERT_TRUE(parsed_traceparent.ok());
  
  auto incoming_tracestate = trace_propagator_.extractTraceState(incoming_context);
  ASSERT_TRUE(incoming_tracestate.has_value());

  // Extract incoming baggage
  auto incoming_baggage = baggage_propagator_.extractBaggage(incoming_context);
  ASSERT_TRUE(incoming_baggage.has_value());
  
  auto parsed_baggage = baggage_propagator_.parseBaggage(incoming_baggage.value());
  ASSERT_TRUE(parsed_baggage.ok());

  // Verify extracted values
  EXPECT_EQ(parsed_traceparent->version, "00");
  EXPECT_EQ(parsed_traceparent->trace_id, "4bf92f3577b34da6a3ce929d0e0e4736");
  EXPECT_EQ(parsed_traceparent->span_id, "00f067aa0ba902b7");
  EXPECT_TRUE(parsed_traceparent->sampled);
  
  EXPECT_EQ(incoming_tracestate.value(), "rojo=00f067aa0ba902b7,congo=t61rcWkgMzE");
  
  ASSERT_EQ(parsed_baggage->size(), 3);
  EXPECT_EQ(parsed_baggage->at("userId").value, "alice");
  EXPECT_EQ(parsed_baggage->at("serverNode").value, "DF%2028");
  EXPECT_EQ(parsed_baggage->at("isProduction").value, "false");

  // Create outgoing context for downstream service
  Tracing::TestTraceContextImpl outgoing_context{};
  
  // Generate new span ID for child span, preserve trace ID
  std::string child_span_id = "b7ad6b7169203331";
  
  // Inject modified trace context
  trace_propagator_.injectTraceParent(outgoing_context, 
                                     parsed_traceparent->version,
                                     parsed_traceparent->trace_id,
                                     child_span_id,
                                     parsed_traceparent->sampled);
  
  // Update tracestate with service information
  std::string updated_tracestate = "envoy=b7ad6b7169203331," + incoming_tracestate.value();
  trace_propagator_.injectTraceState(outgoing_context, updated_tracestate);
  
  // Add baggage for downstream service
  baggage_propagator_.setBaggageValue(outgoing_context, "userId", "alice");
  baggage_propagator_.setBaggageValue(outgoing_context, "requestId", "req-12345");
  baggage_propagator_.setBaggageValue(outgoing_context, "sourceService", "gateway");

  // Verify outgoing headers
  auto outgoing_traceparent = trace_propagator_.extractTraceParent(outgoing_context);
  ASSERT_TRUE(outgoing_traceparent.has_value());
  EXPECT_EQ(outgoing_traceparent.value(), 
           "00-4bf92f3577b34da6a3ce929d0e0e4736-b7ad6b7169203331-01");
  
  auto outgoing_tracestate = trace_propagator_.extractTraceState(outgoing_context);
  ASSERT_TRUE(outgoing_tracestate.has_value());
  EXPECT_EQ(outgoing_tracestate.value(), updated_tracestate);
  
  auto outgoing_baggage_str = baggage_propagator_.extractBaggage(outgoing_context);
  ASSERT_TRUE(outgoing_baggage_str.has_value());
  
  auto outgoing_baggage = baggage_propagator_.parseBaggage(outgoing_baggage_str.value());
  ASSERT_TRUE(outgoing_baggage.ok());
  ASSERT_EQ(outgoing_baggage->size(), 3);
  EXPECT_EQ(outgoing_baggage->at("userId").value, "alice");
  EXPECT_EQ(outgoing_baggage->at("requestId").value, "req-12345");
  EXPECT_EQ(outgoing_baggage->at("sourceService").value, "gateway");
}

TEST_F(W3cIntegrationTest, EmptyToFullPropagation) {
  // Start with empty context (root span scenario)
  Tracing::TestTraceContextImpl context{};
  
  // Create root trace
  std::string root_trace_id = "a1b2c3d4e5f6789012345678901234ab";
  std::string root_span_id = "1234567890abcdef";
  
  trace_propagator_.injectTraceParent(context, "00", root_trace_id, root_span_id, true);
  trace_propagator_.injectTraceState(context, "envoy=1234567890abcdef");
  
  // Add initial baggage
  baggage_propagator_.setBaggageValue(context, "sessionId", "sess-abc123");
  baggage_propagator_.setBaggageValue(context, "feature", "experiment-a");
  
  // Verify headers were created correctly
  EXPECT_TRUE(trace_propagator_.hasTraceParent(context));
  EXPECT_TRUE(baggage_propagator_.hasBaggage(context));
  
  auto traceparent = trace_propagator_.extractTraceParent(context);
  ASSERT_TRUE(traceparent.has_value());
  EXPECT_EQ(traceparent.value(), "00-a1b2c3d4e5f6789012345678901234ab-1234567890abcdef-01");
  
  auto baggage_value = baggage_propagator_.getBaggageValue(context, "sessionId");
  ASSERT_TRUE(baggage_value.ok());
  EXPECT_EQ(baggage_value.value(), "sess-abc123");
}

TEST_F(W3cIntegrationTest, HeaderManipulationOperations) {
  Tracing::TestTraceContextImpl context{
    {"traceparent", "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01"},
    {"baggage", "key1=value1,key2=value2"}
  };
  
  // Test removal operations
  EXPECT_TRUE(trace_propagator_.hasTraceParent(context));
  EXPECT_TRUE(baggage_propagator_.hasBaggage(context));
  
  trace_propagator_.removeTraceParent(context);
  baggage_propagator_.removeBaggage(context);
  
  EXPECT_FALSE(trace_propagator_.hasTraceParent(context));
  EXPECT_FALSE(baggage_propagator_.hasBaggage(context));
  
  // Test re-addition
  trace_propagator_.injectTraceParent(context, "00", 
                                     "4bf92f3577b34da6a3ce929d0e0e4736", 
                                     "00f067aa0ba902b7", false);
  baggage_propagator_.setBaggageValue(context, "newkey", "newvalue");
  
  EXPECT_TRUE(trace_propagator_.hasTraceParent(context));
  EXPECT_TRUE(baggage_propagator_.hasBaggage(context));
  
  // Verify values
  auto traceparent = trace_propagator_.extractTraceParent(context);
  ASSERT_TRUE(traceparent.has_value());
  EXPECT_EQ(traceparent.value(), "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-00");
  
  auto baggage_value = baggage_propagator_.getBaggageValue(context, "newkey");
  ASSERT_TRUE(baggage_value.ok());
  EXPECT_EQ(baggage_value.value(), "newvalue");
}

TEST_F(W3cIntegrationTest, CompatibilityWithExistingTracers) {
  // Test that our propagators work with headers generated by OpenTelemetry
  Tracing::TestTraceContextImpl otel_context{
    {"traceparent", "00-0af7651916cd43dd8448eb211c80319c-b7ad6b7169203331-01"},
    {"tracestate", "otel=b7ad6b7169203331"},
    {"baggage", "tenant=12345,environment=production;property1"}
  };
  
  // Our propagators should be able to parse OpenTelemetry headers
  auto traceparent = trace_propagator_.parseTraceParent(
    trace_propagator_.extractTraceParent(otel_context).value());
  ASSERT_TRUE(traceparent.ok());
  EXPECT_EQ(traceparent->trace_id, "0af7651916cd43dd8448eb211c80319c");
  EXPECT_EQ(traceparent->span_id, "b7ad6b7169203331");
  EXPECT_TRUE(traceparent->sampled);
  
  auto baggage = baggage_propagator_.parseBaggage(
    baggage_propagator_.extractBaggage(otel_context).value());
  ASSERT_TRUE(baggage.ok());
  ASSERT_EQ(baggage->size(), 2);
  EXPECT_EQ(baggage->at("tenant").value, "12345");
  EXPECT_EQ(baggage->at("environment").value, "production");
  EXPECT_EQ(baggage->at("environment").properties.size(), 1);
  EXPECT_EQ(baggage->at("environment").properties[0], "property1");
  
  // Our propagators should generate headers that OpenTelemetry can parse
  Tracing::TestTraceContextImpl envoy_context{};
  
  trace_propagator_.injectTraceParent(envoy_context, "00", 
                                     "0af7651916cd43dd8448eb211c80319c",
                                     "c7ad6b7169203332", true);
  trace_propagator_.injectTraceState(envoy_context, "envoy=c7ad6b7169203332,otel=b7ad6b7169203331");
  
  Baggage::BaggageMap new_baggage;
  Baggage::BaggageMember member;
  member.key = "service";
  member.value = "user-api";
  new_baggage["service"] = member;
  baggage_propagator_.injectBaggage(envoy_context, new_baggage);
  
  // Verify the format is OpenTelemetry-compatible
  auto generated_traceparent = trace_propagator_.extractTraceParent(envoy_context);
  ASSERT_TRUE(generated_traceparent.has_value());
  EXPECT_EQ(generated_traceparent.value(), 
           "00-0af7651916cd43dd8448eb211c80319c-c7ad6b7169203332-01");
  
  auto generated_baggage = baggage_propagator_.extractBaggage(envoy_context);
  ASSERT_TRUE(generated_baggage.has_value());
  EXPECT_EQ(generated_baggage.value(), "service=user-api");
}

} // namespace
} // namespace W3c
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
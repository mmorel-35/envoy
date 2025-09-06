#W3C Propagators Usage Guide

This guide provides comprehensive examples and best practices for using the W3C TraceContext and Baggage propagators in Envoy tracers.

## Basic Usage

### TraceContext Propagator

```cpp
#include "source/extensions/propagators/w3c/tracecontext/tracecontext_propagator.h"

using TraceContextPropagator = Extensions::Propagators::W3c::TraceContext::TraceContextPropagator;

// Create propagator instance
TraceContextPropagator propagator;

// Extract from incoming request
Tracing::TestTraceContextImpl incoming_headers{
    {"traceparent", "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01"},
    {"tracestate", "vendor1=value1,vendor2=value2"}};

auto traceparent_value = propagator.extractTraceParent(incoming_headers);
if (traceparent_value.has_value()) {
  auto parsed = propagator.parseTraceParent(traceparent_value.value());
  if (parsed.ok()) {
    std::string trace_id = parsed->trace_id;
    std::string span_id = parsed->span_id;
    bool sampled = parsed->sampled;
    // Use the extracted values...
  }
}

// Inject for outgoing request
Tracing::TestTraceContextImpl outgoing_headers{};
propagator.injectTraceParent(outgoing_headers, "00", "4bf92f3577b34da6a3ce929d0e0e4736",
                             "b7ad6b7169203331", true);
```

### Baggage Propagator

```cpp
#include "source/extensions/propagators/w3c/baggage/baggage_propagator.h"

using BaggagePropagator = Extensions::Propagators::W3c::Baggage::BaggagePropagator;

// Create propagator instance
BaggagePropagator propagator;

// Extract from incoming request
Tracing::TestTraceContextImpl incoming_headers{
    {"baggage", "userId=alice,sessionId=sess123,feature=experiment-a"}};

auto baggage_value = propagator.extractBaggage(incoming_headers);
if (baggage_value.has_value()) {
  auto baggage_map = propagator.parseBaggage(baggage_value.value());
  if (baggage_map.ok()) {
    for (const auto& [key, member] : *baggage_map) {
      // Process each baggage item
      std::string key_name = member.key;
      std::string value = member.value;
    }
  }
}

// Set individual baggage items
Tracing::TestTraceContextImpl outgoing_headers{};
propagator.setBaggageValue(outgoing_headers, "userId", "alice");
propagator.setBaggageValue(outgoing_headers, "requestId", "req-12345");
```

## Tracer Integration Patterns

### Using with OpenTelemetry

```cpp
// In OpenTelemetry tracer implementation
void injectContext(Tracing::TraceContext& trace_context, const Tracing::UpstreamContext&) override {

  // Use W3C propagators for standardized header injection
  TraceContextPropagator trace_propagator;
  BaggagePropagator baggage_propagator;

  // Inject trace context
  trace_propagator.injectTraceParent(trace_context, kDefaultVersion, getTraceId(), getSpanId(),
                                     sampled());

  if (!getTracestate().empty()) {
    trace_propagator.injectTraceState(trace_context, getTracestate());
  }

  // Inject baggage if present
  if (hasBaggage()) {
    baggage_propagator.injectBaggage(trace_context, getBaggageMap());
  }
}
```

### Class-Level Propagator Usage

```cpp
class CustomTracer {
private:
  TraceContextPropagator trace_propagator_;
  BaggagePropagator baggage_propagator_;

public:
  void extractFromIncomingRequest(const Http::RequestHeaderMap& headers) {
    // Convert headers to TraceContext
    Tracing::HttpTraceContext trace_context(headers);

    // Extract trace context
    if (trace_propagator_.hasTraceParent(trace_context)) {
      auto traceparent = trace_propagator_.extractTraceParent(trace_context);
      auto parsed = trace_propagator_.parseTraceParent(traceparent.value());

      if (parsed.ok()) {
        setTraceId(parsed->trace_id);
        setParentSpanId(parsed->span_id);
        setSampled(parsed->sampled);
      }
    }

    // Extract baggage
    if (baggage_propagator_.hasBaggage(trace_context)) {
      auto user_id = baggage_propagator_.getBaggageValue(trace_context, "userId");
      if (user_id.ok()) {
        setUserId(user_id.value());
      }
    }
  }

  void injectIntoOutgoingRequest(Http::RequestHeaderMap& headers) {
    Tracing::HttpTraceContext trace_context(headers);

    // Inject new span context
    trace_propagator_.injectTraceParent(trace_context, "00", getCurrentTraceId(),
                                        getCurrentSpanId(), isSampled());

    // Propagate baggage
    baggage_propagator_.setBaggageValue(trace_context, "sourceService", "gateway");
  }
};
```

## Advanced Usage

### Baggage with Properties

```cpp
BaggagePropagator propagator;

// Create baggage with properties
BaggageMap baggage_map;
BaggageMember member;
member.key = "userId";
member.value = "alice";
member.properties = {"confidential", "pii"};
baggage_map["userId"] = member;

Tracing::TestTraceContextImpl context{};
propagator.injectBaggage(context, baggage_map);

// Results in: "baggage: userId=alice;confidential;pii"
```

### Tracestate Management

```cpp
TraceContextPropagator propagator;

// Extract existing tracestate
auto existing_tracestate = propagator.extractTraceState(context);

// Add vendor-specific entry
std::string new_tracestate = "envoy=" + current_span_id;
if (existing_tracestate.has_value()) {
  new_tracestate += "," + existing_tracestate.value();
}

// Inject updated tracestate
propagator.injectTraceState(context, new_tracestate);
```

### Error Handling

```cpp
TraceContextPropagator propagator;

auto traceparent = propagator.extractTraceParent(context);
if (traceparent.has_value()) {
  auto parsed = propagator.parseTraceParent(traceparent.value());

  if (!parsed.ok()) {
    // Handle parsing errors
    switch (parsed.status().code()) {
    case absl::StatusCode::kInvalidArgument:
      // Invalid format - log and continue without tracing
      break;
    default:
      // Other errors
      break;
    }
  }
}
```

## Performance Tips

1. **Reuse Propagator Instances**: Propagators are stateless and can be reused as class members
2. **Check Presence First**: Use `hasTraceParent()` and `hasBaggage()` before extraction
3. **Validate Early**: Parse headers early to fail fast on invalid input
4. **Limit Baggage Size**: Be mindful of the 8KB baggage size limit

## Testing Examples

```cpp
TEST(MyTracerTest, W3cPropagation) {
  TraceContextPropagator trace_propagator;
  BaggagePropagator baggage_propagator;

  // Test round-trip
  Tracing::TestTraceContextImpl context{};

  trace_propagator.injectTraceParent(context, "00", "trace123", "span456", true);
  baggage_propagator.setBaggageValue(context, "test", "value");

  auto extracted_traceparent = trace_propagator.extractTraceParent(context);
  EXPECT_TRUE(extracted_traceparent.has_value());

  auto extracted_baggage = baggage_propagator.getBaggageValue(context, "test");
  EXPECT_TRUE(extracted_baggage.ok());
  EXPECT_EQ(extracted_baggage.value(), "value");
}
```

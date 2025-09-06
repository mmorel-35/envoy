# W3C Propagators

This directory contains W3C trace propagation implementations according to the official specifications.

## Overview

The W3C provides two key specifications for distributed tracing:

1. **[W3C Trace Context](https://www.w3.org/TR/trace-context/)** - Standard for trace context propagation across services
2. **[W3C Baggage](https://www.w3.org/TR/baggage/)** - Standard for correlation context propagation

## Components

### TraceContext Propagator

Located in `tracecontext/`, implements the W3C Trace Context specification.

**Headers:**
- `traceparent`: Contains trace context information in format `version-trace-id-parent-id-trace-flags`
- `tracestate`: Contains vendor-specific trace state information

**Features:**
- Extract and inject W3C trace context headers
- Parse and validate traceparent format
- Handle tracestate with multiple vendors
- Full W3C specification compliance
- Support for sampled/unsampled traces

**Example Usage:**
```cpp
#include "source/extensions/propagators/w3c/tracecontext/tracecontext_propagator.h"

TraceContextPropagator propagator;

// Extract trace context
auto traceparent = propagator.extractTraceParent(trace_context);
if (traceparent.has_value()) {
  auto parsed = propagator.parseTraceParent(traceparent.value());
  if (parsed.ok()) {
    // Use parsed->trace_id, parsed->span_id, parsed->sampled
  }
}

// Inject trace context
propagator.injectTraceParent(trace_context, "00", trace_id, span_id, sampled);
propagator.injectTraceState(trace_context, "vendor1=value1,vendor2=value2");
```

### Baggage Propagator

Located in `baggage/`, implements the W3C Baggage specification.

**Headers:**
- `baggage`: Contains key-value pairs in format `key1=value1,key2=value2;property`

**Features:**
- Extract and inject W3C baggage headers
- Parse baggage with properties support
- Validate baggage size and member limits
- Individual key manipulation
- URL encoding support

**Example Usage:**
```cpp
#include "source/extensions/propagators/w3c/baggage/baggage_propagator.h"

BaggagePropagator propagator;

// Extract and parse baggage
auto baggage_str = propagator.extractBaggage(trace_context);
if (baggage_str.has_value()) {
  auto baggage_map = propagator.parseBaggage(baggage_str.value());
  if (baggage_map.ok()) {
    // Use baggage_map->at("key").value
  }
}

// Set individual baggage values
propagator.setBaggageValue(trace_context, "userId", "alice");
propagator.setBaggageValue(trace_context, "serverRegion", "us-west");

// Get specific baggage value
auto user_id = propagator.getBaggageValue(trace_context, "userId");
```

## W3C Specification Compliance

### TraceContext Compliance

- ✅ **Header Format**: Follows exact `version-trace-id-parent-id-trace-flags` format
- ✅ **Field Validation**: Validates field sizes (2-32-16-2 hex characters)
- ✅ **Hex Encoding**: Validates proper hexadecimal encoding
- ✅ **Zero Validation**: Rejects all-zero trace-id and parent-id per spec
- ✅ **Version Support**: Supports version "00" with forward compatibility
- ✅ **Trace Flags**: Proper parsing and encoding of sampled flag (bit 0)
- ✅ **Tracestate**: Multiple vendor support with comma separation
- ✅ **Case Sensitivity**: Handles lowercase hex as per specification

### Baggage Compliance

- ✅ **Header Format**: Follows `key=value,key2=value2` format
- ✅ **Size Limits**: Enforces 8KB total size limit
- ✅ **Member Limits**: Enforces practical 180 member limit
- ✅ **Key Validation**: Validates key characters per specification
- ✅ **Value Validation**: Validates value encoding
- ✅ **Properties**: Supports baggage member properties
- ✅ **URL Encoding**: Handles URL-encoded values
- ✅ **Whitespace**: Proper whitespace handling

## Testing

Both propagators have comprehensive test suites covering:

- **Happy Path**: Standard usage scenarios
- **Edge Cases**: Boundary conditions and limits
- **Error Handling**: Invalid input validation
- **Round Trip**: Extract/inject consistency
- **W3C Compliance**: Specification adherence
- **Performance**: Efficient string handling

### Running Tests

```bash
# Build and test TraceContext propagator
bazel test //test/extensions/propagators/w3c/tracecontext:tracecontext_propagator_test

# Build and test Baggage propagator  
bazel test //test/extensions/propagators/w3c/baggage:baggage_propagator_test

# Run all W3C propagator tests
bazel test //test/extensions/propagators/w3c/...
```

## Integration

These propagators integrate with Envoy's tracing infrastructure through the `TraceContextHandler` system:

1. **Constants**: Provide header name constants for trace context manipulation
2. **Extraction**: Parse incoming headers into structured data
3. **Injection**: Serialize structured data into outgoing headers
4. **Validation**: Ensure compliance with W3C specifications

The propagators are designed to be used by tracers like OpenTelemetry, Zipkin, and others to provide standardized W3C header handling.

## Performance Considerations

- **Memory Efficient**: Uses reference semantics where possible
- **Minimal Allocations**: Reuses string views for parsing
- **Early Validation**: Fails fast on invalid input
- **Size Limits**: Prevents unbounded memory usage

## Standards References

- [W3C Trace Context Specification](https://www.w3.org/TR/trace-context/)
- [W3C Baggage Specification](https://www.w3.org/TR/baggage/)
- [OpenTelemetry Trace Context](https://opentelemetry.io/docs/specs/otel/context/api-propagators/)
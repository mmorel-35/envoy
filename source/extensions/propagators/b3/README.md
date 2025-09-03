# B3 Propagator

This directory contains a comprehensive B3 propagator implementation that provides reusable B3 distributed tracing support for Envoy extensions.

## Overview

The B3 propagator implements the [B3 Propagation specification](https://github.com/openzipkin/b3-propagation) and provides a clean, reusable interface for B3 header extraction and injection across all Envoy tracers.

## Problem

Currently, B3 trace context parsing logic is implemented only in the Zipkin tracer, with ~200+ lines of duplicated parsing code that includes:
- B3 header parsing constants and validation logic
- Hex validation and format checking functions  
- Complex single header format parsing
- Sampling state management

Other tracers cannot easily add B3 support, leading to fragmented B3 implementation across the codebase.

## Solution

This B3 propagator provides:

### B3-Compliant Data Structures
- **`TraceId`**: Handles 64-bit and 128-bit trace IDs with proper validation
- **`SpanId`**: Manages 64-bit span IDs with hex validation
- **`SamplingState`**: Represents B3 sampling decisions (not sampled, sampled, debug, unspecified)
- **`TraceContext`**: Complete B3 context combining all trace propagation data

### Reusable Propagation Interface
- **`Propagator`**: Main interface for extracting and injecting B3 headers
- **`TracingHelper`**: Backward compatibility layer for existing tracers
- Support for both multiple headers and single header formats

## Key Features

- ✅ Full B3 specification compliance for both multiple and single header formats
- ✅ 64-bit and 128-bit trace ID support with proper validation
- ✅ Sampling state management including debug sampling
- ✅ Comprehensive validation of encoding, field lengths, and format structure
- ✅ Clean separation between data structures and business logic
- ✅ Uses official B3 terminology throughout
- ✅ Backward compatibility for existing tracers

## Usage Examples

### Enhanced Tracer Integration

```cpp
// Before: Zipkin tracer has ~200 lines of B3 parsing logic
constexpr char X_B3_TRACE_ID[] = "x-b3-traceid";
bool parseHexString(const std::string& hex_str, uint64_t& result) { /* duplicated */ }
std::pair<SpanContext, bool> extractSpanContext() { /* ~200 lines of B3 logic */ }

// After: Use the reusable B3 propagator
using B3 = Extensions::Propagators::B3;

bool hasB3Headers(const Tracing::TraceContext& trace_context) {
  return B3::Propagator::isPresent(trace_context);
}

absl::StatusOr<SpanContext> extractSpanContext(const Tracing::TraceContext& trace_context) {
  auto extracted = B3::TracingHelper::extractForTracer(trace_context);
  if (!extracted.has_value()) {
    return absl::InvalidArgumentError("No B3 trace context found");
  }
  return convertFromB3(extracted.value());
}
```

### Direct B3 Operations

```cpp
// Extract B3 context from headers
auto b3_context = B3::Propagator::extract(trace_context);
if (b3_context.ok()) {
  const auto& context = b3_context.value();
  
  // Access B3 components
  std::string trace_id = context.traceId().toHexString();  // 64 or 128-bit
  std::string span_id = context.spanId().toHexString();    // 64-bit
  bool sampled = context.sampled();                        // Including debug sampling
  
  // Create new B3 context
  auto new_context = B3::TracingHelper::createTraceContext(
      0,           // trace_id_high (0 for 64-bit)
      0x123456789abcdef, // trace_id_low
      0x987654321fedcba,  // span_id
      0x555555555555555,  // parent_span_id
      true                // sampled
  );
  
  // Inject back into headers
  B3::Propagator::inject(new_context, trace_context);
}
```

## Supported Formats

### Multiple Headers Format
```
x-b3-traceid: 80f198ee56343ba864fe8b2a57d3eff7
x-b3-spanid: e457b5a2e4d86bd1
x-b3-parentspanid: 05e3ac9a4f6e3b90
x-b3-sampled: 1
x-b3-flags: 1
```

### Single Header Format
```
b3: 80f198ee56343ba864fe8b2a57d3eff7-e457b5a2e4d86bd1-1-05e3ac9a4f6e3b90
```

## Benefits

- **Eliminates ~200 lines of duplicated B3 code** from zipkin tracer
- **Enables B3 support in all tracers** (OpenTelemetry, Datadog, etc.)
- **Ensures B3 specification compliance** across all Envoy tracers
- **Reduces maintenance burden** with centralized logic
- **Provides backward compatibility** for smooth migration
- **Supports both B3 formats** for maximum interoperability

## Testing

The implementation includes comprehensive testing that validates:
- B3 specification compliance for both single and multiple header formats
- 64-bit and 128-bit trace ID handling
- Sampling state management including debug sampling
- Round-trip serialization/deserialization
- Error handling for invalid formats and edge cases
- Integration with Envoy's trace context system
- Tracer helper compatibility

This provides the foundation for eliminating B3 parsing duplication across tracers while ensuring full B3 specification compliance.
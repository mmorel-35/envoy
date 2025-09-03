# B3 Propagator

This directory contains a comprehensive B3 propagator implementation that provides reusable B3 distributed tracing support for Envoy extensions with full compliance to the [B3 Propagation specification](https://github.com/openzipkin/b3-propagation).

## Overview

The B3 propagator implements the complete B3 specification and provides a clean, reusable interface for B3 header extraction and injection across all Envoy tracers, eliminating code duplication while ensuring specification compliance.

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

## Specification Compliance

### Complete B3 Propagation Specification Support
- ✅ **Multiple Headers Format**: Full support for `x-b3-traceid`, `x-b3-spanid`, `x-b3-parentspanid`, `x-b3-sampled`, `x-b3-flags`
- ✅ **Single Header Format**: Complete `b3` header support with format `{traceId}-{spanId}-{sampled}-{parentSpanId}`
- ✅ **Header Case Insensitivity**: Handles `X-B3-TraceId`, `x-b3-traceid`, `X-B3-TRACEID` correctly
- ✅ **64-bit and 128-bit Trace IDs**: Proper handling of both trace ID formats with validation
- ✅ **Zero ID Rejection**: Rejects all-zero trace IDs and span IDs per B3 specification
- ✅ **Sampling States**: Complete support for "0" (not sampled), "1" (sampled), "d" (debug), and case-insensitive "true"/"false"
- ✅ **Hex Validation**: Strict validation of trace IDs and span IDs as proper hex strings
- ✅ **Parent Span Handling**: Optional parent span ID support for distributed tracing chains

### Format Detection and Compatibility
- **Automatic Format Detection**: Intelligently detects single vs multiple header formats
- **Format-Specific Injection**: Respects original format preference when re-injecting
- **Backward Compatibility**: Seamless integration with existing B3-based tracers
- **Error Tolerance**: Graceful handling of malformed headers while preserving valid data

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

The implementation includes comprehensive testing that validates complete B3 specification compliance:

### B3 Specification Compliance Testing
- **Multiple Headers Format**: Complete validation of all x-b3-* header combinations
- **Single Header Format**: Full b3 header parsing and format validation
- **Header Case Insensitivity**: Mixed case header name handling per HTTP specification
- **Trace ID Formats**: Both 64-bit and 128-bit trace ID extraction and validation
- **Sampling State Handling**: All sampling values including "0", "1", "d", "true", "false" (case-insensitive)
- **Zero ID Rejection**: Proper rejection of invalid zero trace IDs and span IDs
- **Hex Validation**: Invalid hex string detection and error handling

### Integration and Compatibility Testing
- **Format Detection**: Automatic detection between single and multiple header formats
- **Round-trip Consistency**: Extraction followed by injection maintains data integrity  
- **Error Handling**: Robust handling of malformed headers and invalid formats
- **Tracer Compatibility**: Integration testing with Envoy's trace context system
- **Performance**: Efficient parsing and injection with minimal overhead

### Edge Case Coverage
- **Partial Headers**: Handling of incomplete header sets gracefully
- **Mixed Formats**: Proper priority when both single and multiple headers present
- **Parent Span Handling**: Optional parent span ID processing
- **Debug Sampling**: Special handling of debug sampling flag combinations

This provides the foundation for eliminating B3 parsing duplication across tracers while ensuring full B3 specification compliance.
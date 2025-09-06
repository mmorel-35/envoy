# W3C TraceContext Propagator

This implements the [W3C Trace Context specification](https://www.w3.org/TR/trace-context/) for distributed tracing header propagation.

## Overview

The W3C TraceContext specification defines standard HTTP headers for propagating trace context information across services in a distributed system:

- **`traceparent`**: Contains the core trace context in format `version-trace-id-parent-id-trace-flags`
- **`tracestate`**: Contains vendor-specific trace information as key-value pairs

## Features

- ✅ **Full W3C Compliance**: Implements the complete W3C Trace Context specification
- ✅ **Header Validation**: Validates all components per specification requirements
- ✅ **Format Checking**: Ensures proper hex encoding and field sizes
- ✅ **Sampled Flag Support**: Handles trace sampling decision propagation
- ✅ **Tracestate Support**: Manages vendor-specific trace state information
- ✅ **Error Handling**: Robust parsing with detailed error messages

## Usage

### Basic Usage

```cpp
#include "source/extensions/propagators/w3c/tracecontext/tracecontext_propagator.h"

TraceContextPropagator propagator;

// Extract traceparent from incoming request
auto traceparent = propagator.extractTraceParent(trace_context);
if (traceparent.has_value()) {
  auto parsed = propagator.parseTraceParent(traceparent.value());
  if (parsed.ok()) {
    // Access parsed components
    std::string version = parsed->version;     // "00"
    std::string trace_id = parsed->trace_id;   // 32 hex characters
    std::string span_id = parsed->span_id;     // 16 hex characters  
    bool sampled = parsed->sampled;            // sampling decision
  }
}

// Inject traceparent for outgoing request
propagator.injectTraceParent(trace_context, "00", trace_id, span_id, sampled);
```

### Advanced Usage with Tracestate

```cpp
// Extract tracestate header
auto tracestate = propagator.extractTraceState(trace_context);
if (tracestate.has_value()) {
  // Process vendor-specific trace state
  // Format: "vendor1=value1,vendor2=value2"
}

// Inject tracestate
propagator.injectTraceState(trace_context, "myvendor=abc123,othertool=xyz789");
```

### Utility Methods

```cpp
// Check if traceparent is present
if (propagator.hasTraceParent(trace_context)) {
  // Process existing trace context
}

// Remove headers
propagator.removeTraceParent(trace_context);
propagator.removeTraceState(trace_context);
```

## W3C Specification Compliance

### Traceparent Header Format

The `traceparent` header follows this exact format:
```
version-trace-id-parent-id-trace-flags
```

**Components:**
- `version`: 2 hex characters (currently "00")
- `trace-id`: 32 hex characters (128-bit trace ID)
- `parent-id`: 16 hex characters (64-bit parent span ID)  
- `trace-flags`: 2 hex characters (8-bit flags, bit 0 = sampled)

**Example:**
```
00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01
```

### Validation Rules

✅ **Header Length**: Must be exactly 55 characters
✅ **Component Count**: Must have exactly 4 hyphen-separated parts
✅ **Field Sizes**: Version(2), trace-id(32), parent-id(16), flags(2) hex chars
✅ **Hex Encoding**: All components must be valid lowercase hexadecimal
✅ **Zero Validation**: trace-id and parent-id cannot be all zeros
✅ **Version Support**: Supports version "00" with forward compatibility

### Tracestate Header

The `tracestate` header contains vendor-specific data:
```
vendor1=value1,vendor2=value2
```

**Features:**
- Multiple vendor entries separated by commas
- Order preservation (most recent first)
- Size limits enforced (512 characters max per vendor)
- Proper comma and equals sign handling

## Error Handling

The propagator provides detailed error messages for invalid input:

```cpp
auto result = propagator.parseTraceParent("invalid-header");
if (!result.ok()) {
  // Error details include:
  // - Expected vs actual field sizes
  // - Specific validation failures
  // - W3C specification references
  std::string error = result.status().message();
}
```

## Performance Considerations

- **Memory Efficient**: Uses string views during parsing to minimize allocations
- **Early Validation**: Fails fast on invalid input without unnecessary processing
- **Optimized Injection**: Minimal string operations for header generation

## Integration

This propagator integrates with Envoy's tracing infrastructure through:

1. **Header Constants**: Provides `W3cConstants` for consistent header access
2. **TraceContext API**: Uses standard `Tracing::TraceContext` interface
3. **Status Handling**: Returns `absl::StatusOr` for robust error handling

## Testing

Comprehensive test coverage includes:
- Valid/invalid header parsing
- Edge cases and boundary conditions
- Round-trip extraction/injection
- W3C specification compliance
- Performance benchmarks

Run tests with:
```bash
bazel test //test/extensions/propagators/w3c/tracecontext:tracecontext_propagator_test
```

## Standards References

- [W3C Trace Context Specification](https://www.w3.org/TR/trace-context/)
- [W3C Trace Context GitHub Repository](https://github.com/w3c/trace-context)
- [OpenTelemetry Trace Context](https://opentelemetry.io/docs/specs/otel/context/api-propagators/)
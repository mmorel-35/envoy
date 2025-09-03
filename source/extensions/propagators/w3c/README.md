# W3C Trace Context Propagator

This component provides a reusable implementation of the [W3C Trace Context specification](https://www.w3.org/TR/trace-context/) for Envoy.

## Overview

The W3C Trace Context propagator separates the data structures from the business logic and provides a clean interface for extracting and injecting W3C trace context headers. It is designed to be compliant with the official W3C specification and uses the official terminology.

## Components

### Data Structures

- `TraceParent`: Represents the traceparent header value with version, trace-id, parent-id, and trace-flags
- `TraceState`: Represents the tracestate header value with vendor-specific key-value pairs  
- `TraceContext`: Complete W3C trace context containing both traceparent and tracestate

### Propagator Interface

- `Propagator`: Main interface for extracting and injecting W3C trace context headers
- `TracingHelper`: Utility class for backward compatibility with existing Envoy tracers

## Usage

### Basic Extraction

```cpp
#include "source/extensions/propagators/w3c/propagator.h"

// Check if W3C headers are present
if (Propagator::isPresent(trace_context)) {
  // Extract W3C trace context
  auto result = Propagator::extract(trace_context);
  if (result.ok()) {
    const auto& w3c_context = result.value();
    // Use the extracted context...
  }
}
```

### Basic Injection

```cpp
// Create or obtain a W3C trace context
TraceContext w3c_context = ...;

// Inject into headers
Propagator::inject(w3c_context, trace_context);
```

### Creating New Contexts

```cpp
// Create a root trace context
auto root = Propagator::createRoot("4bf92f3577b34da6a3ce929d0e0e4736", 
                                   "00f067aa0ba902b7", 
                                   true /* sampled */);

// Create a child context
auto child = Propagator::createChild(parent_context, "b7ad6b7169203331");
```

### Backward Compatibility

```cpp
// For existing tracers that need extracted values
auto extracted = TracingHelper::extractForTracer(trace_context);
if (extracted.has_value()) {
  const auto& values = extracted.value();
  // values.version, values.trace_id, values.span_id, etc.
}
```

## Compliance

This implementation follows the W3C Trace Context specification:

- Proper traceparent format validation (version-trace-id-parent-id-trace-flags)
- Tracestate handling with vendor-specific key-value pairs
- Correct handling of sampling flags
- Validation of hex encoding and field lengths
- Proper error handling for invalid formats

## Future Extensibility

The design prepares for future W3C specifications:

- Structure supports adding W3C Baggage propagation
- Clean separation between data and logic
- Extensible interfaces for new header types

## Testing

Comprehensive tests validate:

- W3C specification compliance
- Round-trip serialization/deserialization  
- Error handling for invalid inputs
- Integration with Envoy's trace context system
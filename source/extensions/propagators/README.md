# Trace Propagators

This directory contains the implementation of trace propagators for Envoy, providing a clean and modular way to handle different trace context propagation formats.

## Overview

Trace propagators are responsible for extracting and injecting trace context information from/to trace contexts (typically HTTP headers). This implementation provides a unified interface and shared constants to support multiple propagation formats.

## Supported Propagators

### B3 Propagator (`b3/`)
Implements the B3 propagation specification used by Zipkin and other tracing systems.

**Formats supported:**
- Multi-header format: `X-B3-TraceId`, `X-B3-SpanId`, `X-B3-ParentSpanId`, `X-B3-Sampled`, `X-B3-Flags`
- Single header format: `b3: {TraceId}-{SpanId}-{SamplingState}-{ParentSpanId}`

**Reference:** https://github.com/openzipkin/b3-propagation

### W3C Propagator (`w3c/`)
Implements the W3C Trace Context specification.

**Headers:**
- `traceparent`: `00-{trace-id}-{parent-id}-{trace-flags}`
- `tracestate`: `key1=value1,key2=value2` (optional vendor-specific state)

**Reference:** https://www.w3.org/TR/trace-context/

### X-Ray Propagator (`xray/`)
Implements the AWS X-Ray trace context format.

**Header:**
- `X-Amzn-Trace-Id`: `Root={trace-id};Parent={parent-id};Sampled={0|1}`

**Reference:** https://docs.aws.amazon.com/xray/latest/devguide/xray-concepts.html

## Architecture

### Core Components

1. **Propagator Interface** (`propagator_interface.h`)
   - Base interface for all propagators
   - Defines `extract()` and `inject()` methods
   - Uses `TraceHeader` struct for common trace information

2. **Shared Constants** (`propagator_constants.h`)
   - Centralized header constants to avoid duplication
   - Uses `TraceContextHandler` for optimized header access
   - Singleton pattern for efficient access

3. **Individual Propagators**
   - Self-contained implementations for each format
   - Format-specific validation and parsing
   - Proper error handling and fallbacks

### Key Design Principles

- **Minimal and focused**: Skeleton implementation for progressive development
- **Shared constants**: Avoid duplication across propagators
- **Clean interfaces**: Simple extract/inject pattern
- **Extensible**: Easy to add new propagation formats
- **Consistent**: Follows Envoy patterns and conventions

## Usage Example

```cpp
#include "source/extensions/propagators/b3/b3_propagator.h"
#include "source/extensions/propagators/w3c/w3c_propagator.h"

// Create propagators
auto b3_propagator = std::make_unique<B3::B3Propagator>();
auto w3c_propagator = std::make_unique<W3C::W3CPropagator>();

// Extract trace context
auto trace_header = b3_propagator->extract(trace_context);

// Inject into different format
w3c_propagator->inject(trace_context, trace_header);
```

## Future Extensions

The skeleton is designed to be easily extended with:

- Jaeger propagation format
- Baggage propagation
- Custom vendor-specific formats
- Configuration-driven propagator selection
- Chained propagator support

## Development Notes

This is a skeleton implementation focusing on:
- Clean architecture and interfaces
- Shared infrastructure (constants, validation)
- Basic functionality for each propagator
- Proper namespace organization
- Comprehensive documentation

Future work should focus on:
- Complete implementation of all methods
- Comprehensive testing
- Performance optimization
- Integration with Envoy's tracing system
- Configuration and factory patterns
#W3C Propagators

This directory contains W3C trace propagation implementations according to the official specifications, providing standardized distributed tracing header support for Envoy.

## Overview

The W3C provides two key specifications for distributed tracing:

1. **[W3C Trace Context](https://www.w3.org/TR/trace-context/)** - Standard for trace context propagation across services
2. **[W3C Baggage](https://www.w3.org/TR/baggage/)** - Standard for correlation context propagation

## Components

### TraceContext Propagator

Located in `tracecontext/`, implements the W3C Trace Context specification for `traceparent` and `tracestate` headers. See [TraceContext README](tracecontext/README.md) for detailed specification compliance.

### Baggage Propagator

Located in `baggage/`, implements the W3C Baggage specification for `baggage` headers. See [Baggage README](baggage/README.md) for detailed specification compliance.

## Integration with Envoy Tracers

These propagators are designed as centralized components used by all Envoy tracers:

- **OpenTelemetry**: Uses `TraceContextPropagator` for W3C header handling
- **Zipkin**: Uses `TraceContextPropagator` for W3C fallback support  
- **Fluentd**: Uses `TraceContextPropagator` for trace context extraction and injection

This centralized approach eliminates code duplication and ensures consistent W3C behavior across all tracing implementations.

## Usage

For comprehensive usage examples, integration patterns, and advanced features, see [USAGE.md](USAGE.md).

## Quick Start

```cpp
#include "source/extensions/propagators/w3c/tracecontext/tracecontext_propagator.h"
#include "source/extensions/propagators/w3c/baggage/baggage_propagator.h"

// Extract trace context
TraceContextPropagator trace_propagator;
auto traceparent = trace_propagator.extractTraceParent(trace_context);

// Handle baggage
BaggagePropagator baggage_propagator;
baggage_propagator.setBaggageValue(trace_context, "userId", "alice");
```

## Testing

```bash
#Run all W3C propagator tests
bazel test //test/extensions/propagators/w3c/...
```

## Standards References

- [W3C Trace Context Specification](https://www.w3.org/TR/trace-context/)
- [W3C Baggage Specification](https://www.w3.org/TR/baggage/)

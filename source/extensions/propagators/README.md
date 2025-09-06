# Envoy Propagators

This directory contains comprehensive trace propagation implementations for distributed tracing in Envoy, supporting multiple industry-standard protocols and formats.

## Overview

Envoy's propagator implementations provide standardized distributed tracing header support, enabling seamless trace context propagation across microservices. Each propagator is designed for full specification compliance, high performance, and easy integration with Envoy's tracing infrastructure.

## Directory Structure

The directory is organized by protocol, with variants in subdirectories when multiple formats exist:

```
propagators/
├── README.md                           # This overview document
├── SPECIFICATION_COMPLIANCE.md        # Detailed compliance matrices
├── TESTING_GUIDE.md                   # Comprehensive testing guide
├── INTEGRATION_EXAMPLES.md            # Real-world integration patterns
├── b3/                                # B3 trace propagation (Zipkin)
│   ├── multi/                         # Multi-header B3 format
│   │   ├── b3_multi_propagator.h/cc
│   │   └── BUILD
│   └── single/                        # Single-header B3 format
│       ├── b3_single_propagator.h/cc
│       └── BUILD
├── w3c/                               # W3C trace propagation standards
│   ├── README.md                      # W3C propagators overview
│   ├── USAGE.md                       # Detailed usage guide
│   ├── tracecontext/                  # W3C TraceContext specification
│   │   ├── tracecontext_propagator.h/cc
│   │   ├── README.md
│   │   └── BUILD
│   └── baggage/                       # W3C Baggage specification
│       ├── baggage_propagator.h/cc
│       ├── README.md
│       └── BUILD
├── skywalking/                        # Apache SkyWalking propagation
│   ├── skywalking_propagator.h/cc
│   └── BUILD
└── xray/                              # AWS X-Ray propagation
    ├── xray_propagator.h/cc
    └── BUILD
```

## Supported Protocols

### W3C Standards (Recommended)

- **[W3C TraceContext](w3c/tracecontext/)** - Standard for trace context propagation via `traceparent` and `tracestate` headers
- **[W3C Baggage](w3c/baggage/)** - Standard for correlation context propagation via `baggage` headers

These are the modern standards recommended for new deployments. [Full W3C documentation →](w3c/)

### B3 Propagation (Zipkin)

- **[B3 Multi-Header](b3/multi/)** - Original B3 format with separate headers (`X-B3-TraceId`, `X-B3-SpanId`, etc.)
- **[B3 Single-Header](b3/single/)** - Compact B3 format using single `b3` header

Widely used in Zipkin-based tracing systems and legacy environments.

### Vendor-Specific

- **[Apache SkyWalking](skywalking/)** - Native `sw8` protocol for SkyWalking APM
- **[AWS X-Ray](xray/)** - Native `X-Amzn-Trace-Id` header for AWS X-Ray tracing

## Documentation

### Quick Links

- 📋 **[Specification Compliance](SPECIFICATION_COMPLIANCE.md)** - Detailed compliance matrices for all propagators
- 🧪 **[Testing Guide](TESTING_GUIDE.md)** - Comprehensive testing strategies and commands
- 🔧 **[W3C Usage Guide](w3c/USAGE.md)** - Detailed examples and integration patterns
- 🔗 **[Integration Examples](INTEGRATION_EXAMPLES.md)** - Real-world tracer integration patterns and examples

### Individual Propagator Documentation

Each propagator includes comprehensive documentation:

- **README.md** - Overview, features, and basic usage
- **Implementation details** - Specification compliance and validation rules
- **Integration examples** - Real-world usage patterns with Envoy tracers

## Quick Start

### Basic W3C TraceContext Usage

```cpp
#include "source/extensions/propagators/w3c/tracecontext/tracecontext_propagator.h"

using TraceContextPropagator = Extensions::Propagators::W3c::TraceContext::TraceContextPropagator;

// Extract from incoming request
TraceContextPropagator propagator;
auto traceparent = propagator.extractTraceParent(trace_context);
if (traceparent.has_value()) {
  auto parsed = propagator.parseTraceParent(traceparent.value());
  // Use parsed trace context...
}

// Inject for outgoing request
propagator.injectTraceParent(trace_context, "00", trace_id, span_id, sampled);
```

### Basic Baggage Usage

```cpp
#include "source/extensions/propagators/w3c/baggage/baggage_propagator.h"

using BaggagePropagator = Extensions::Propagators::W3c::Baggage::BaggagePropagator;

BaggagePropagator propagator;

// Set correlation data
propagator.setBaggageValue(trace_context, "userId", "alice");
propagator.setBaggageValue(trace_context, "feature", "newUI");

// Extract in downstream service
auto user_id = propagator.getBaggageValue(trace_context, "userId");
```

## Design Principles

### 1. Specification Compliance

All propagators are designed for **100% specification compliance** with their respective standards:

- W3C TraceContext and Baggage specifications
- Zipkin B3 propagation formats
- Vendor-specific protocols (X-Ray, SkyWalking)

### 2. Performance Optimized

- **Memory efficient**: Minimal allocations during parsing and injection
- **CPU efficient**: Optimized string operations and validation
- **Zero-copy where possible**: String views for parsing operations

### 3. Robust Error Handling

- **Detailed error messages** with specification references
- **Graceful degradation** for invalid or missing headers
- **Fail-fast validation** to prevent resource exhaustion

### 4. Tracer Integration

Designed as **centralized components** used by all Envoy tracers:

- **OpenTelemetry**: Uses W3C propagators for standard compliance
- **Zipkin**: Uses both B3 and W3C propagators
- **Fluentd**: Uses W3C propagators for trace context handling

This eliminates code duplication and ensures consistent behavior across tracing implementations.

## File Naming Convention

All propagator files follow a standardized naming convention:

- **Header files**: `<protocol>_<variant>_propagator.h`
- **Implementation files**: `<protocol>_<variant>_propagator.cc`  
- **Test files**: `<protocol>_<variant>_propagator_test.cc`
- **Build targets**: `<protocol>_<variant>_propagator_lib`

### Examples

- `tracecontext_propagator.h/.cc` - W3C TraceContext implementation
- `baggage_propagator.h/.cc` - W3C Baggage implementation
- `b3_multi_propagator.h/.cc` - B3 multi-header format
- `b3_single_propagator.h/.cc` - B3 single-header format
- `skywalking_propagator.h/.cc` - SkyWalking sw8 protocol
- `xray_propagator.h/.cc` - AWS X-Ray trace headers

## Build and Test

### Build All Propagators

```bash
# Build all propagator libraries
bazel build //source/extensions/propagators/...

# Build specific propagator
bazel build //source/extensions/propagators/w3c/tracecontext:tracecontext_propagator_lib
```

### Run Tests

```bash
# Run all propagator tests
bazel test //test/extensions/propagators/...

# Run W3C propagator tests only
bazel test //test/extensions/propagators/w3c/...

# Run with coverage
bazel coverage //test/extensions/propagators/...
```

For detailed testing instructions, see the [Testing Guide](TESTING_GUIDE.md).

## Integration with Envoy

### Tracer Usage

Propagators integrate seamlessly with Envoy's tracing infrastructure:

```cpp
// Example: Using propagators in a custom tracer
class MyTracer {
private:
  TraceContextPropagator trace_propagator_;
  BaggagePropagator baggage_propagator_;

public:
  void extractContext(const Http::RequestHeaderMap& headers) {
    Tracing::HttpTraceContext trace_context(headers);
    
    // Extract W3C trace context
    if (trace_propagator_.hasTraceParent(trace_context)) {
      auto traceparent = trace_propagator_.extractTraceParent(trace_context);
      // Process trace context...
    }
    
    // Extract baggage for correlation
    auto user_id = baggage_propagator_.getBaggageValue(trace_context, "userId");
  }
};
```

### Configuration

Propagators are configured automatically based on tracer selection. See individual tracer documentation for specific configuration options.

## Contributing

When contributing to propagator implementations:

1. **Follow specification compliance** - Implement the complete specification
2. **Add comprehensive tests** - Include unit, integration, and compliance tests
3. **Document thoroughly** - Provide clear documentation and examples
4. **Performance testing** - Ensure no regressions in critical paths
5. **Cross-propagator testing** - Test interoperability with existing propagators

## Support and Resources

- **Specifications**: Links to official specifications in individual READMEs
- **Issues**: Report bugs or feature requests in the main Envoy repository
- **Testing**: Comprehensive test coverage ensures reliability and compliance
- **Performance**: Regular benchmarking ensures optimal performance

For detailed usage patterns and advanced features, see the [W3C Usage Guide](w3c/USAGE.md).

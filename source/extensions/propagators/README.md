# Envoy Distributed Tracing Propagators

This directory contains comprehensive, specification-compliant implementations of distributed tracing propagators for Envoy. These propagators provide unified, reusable interfaces for handling multiple trace formats while eliminating code duplication across tracers.

## Overview

The propagator implementations solve the problem of duplicated trace parsing logic across Envoy tracers by providing centralized, specification-compliant propagation support for:

- **W3C Trace Context** - World Wide Web Consortium standard for trace propagation
- **W3C Baggage** - W3C standard for distributed context propagation  
- **B3 Propagation** - Zipkin's B3 distributed tracing format
- **OpenTelemetry Composite** - OpenTelemetry's multi-format propagator specification

## Key Benefits

🔧 **Eliminates Code Duplication**
- Removes ~200 lines of duplicate parsing logic per tracer
- Centralizes validation and format handling in reusable components

🔧 **Ensures Specification Compliance**
- Implements complete official specifications with comprehensive test coverage
- Handles edge cases, error conditions, and future version compatibility

🔧 **Enables Multi-Format Support**
- Allows all tracers to support multiple trace formats (W3C, B3, etc.)
- Provides automatic format detection and conversion capabilities

🔧 **Provides Complete Baggage Support**
- Implements full W3C Baggage specification with size limits and validation
- Integrates with Envoy's standard Span baggage interface

🔧 **Maintains Backward Compatibility**
- Drop-in replacements for existing tracer parsing logic
- Smooth migration path with minimal code changes required

## Directory Structure

```
propagators/
├── README.md                           # This overview document
├── SPECIFICATION_COMPLIANCE.md         # Detailed specification compliance documentation
├── TESTING_GUIDE.md                   # Comprehensive testing strategy and guidelines
├── w3c/                               # W3C Trace Context and Baggage propagator
│   ├── README.md                      # W3C-specific documentation
│   ├── propagator.h/.cc               # Main W3C propagator implementation
│   ├── trace_context.h/.cc            # W3C trace context data structures
│   └── integration_example.h          # Integration examples for tracers
├── b3/                                # B3 Propagation propagator
│   ├── README.md                      # B3-specific documentation  
│   ├── propagator.h/.cc               # Main B3 propagator implementation
│   ├── trace_context.h/.cc            # B3 trace context data structures
│   └── integration_example.h          # B3 integration examples
└── opentelemetry/                     # OpenTelemetry Composite propagator
    ├── README.md                      # OpenTelemetry composite documentation
    ├── propagator.h/.cc               # Main composite propagator implementation
    ├── trace_context.h/.cc            # Composite trace context structures
    └── integration_example.h          # Composite propagator examples
```

## Quick Start Guide

### 1. Using W3C Propagator

```cpp
#include "source/extensions/propagators/w3c/propagator.h"

using W3C = Extensions::Propagators::W3C;

// Check if W3C headers are present
if (W3C::Propagator::isPresent(trace_context)) {
  // Extract W3C context
  auto result = W3C::Propagator::extract(trace_context);
  if (result.ok()) {
    const auto& context = result.value();
    // Use extracted context...
  }
}

// Inject W3C headers
W3C::TraceContext w3c_context = ...;
W3C::Propagator::inject(w3c_context, trace_context);
```

### 2. Using B3 Propagator

```cpp
#include "source/extensions/propagators/b3/propagator.h"

using B3 = Extensions::Propagators::B3;

// Extract B3 context (supports both single and multiple header formats)
auto result = B3::Propagator::extract(trace_context);
if (result.ok()) {
  const auto& context = result.value();
  std::string trace_id = context.traceId().toHexString();
  bool sampled = context.sampled();
}

// For tracer integration
auto extracted = B3::TracingHelper::extractForTracer(trace_context);
if (extracted.has_value()) {
  // Convert to your tracer's format
}
```

### 3. Using OpenTelemetry Composite Propagator

```cpp
#include "source/extensions/propagators/opentelemetry/propagator.h"

using OpenTelemetry = Extensions::Propagators::OpenTelemetry;

// Configure propagators (supports OTEL_PROPAGATORS environment variable)
std::vector<std::string> propagator_strings = {"tracecontext", "b3", "baggage"};
auto service = std::make_unique<PropagatorService>(propagator_strings);

// Extract from any supported format (priority-based)
auto result = service->extract(trace_context);
if (result.ok()) {
  auto composite_context = result.value();
  // Works with W3C or B3 input
}

// Inject to all configured formats simultaneously
service->inject(composite_context, trace_context);
```

## Migration Guide

### From Duplicate Tracer Logic to Propagators

**Before** (in each tracer):
```cpp
// ~200 lines of duplicated parsing code per tracer
constexpr int kTraceparentHeaderSize = 55;
bool isValidHex(const absl::string_view& input) { /* duplicated */ }
class SpanContextExtractor {
  absl::StatusOr<SpanContext> extractSpanContext() { /* duplicated logic */ }
};
```

**After** (using propagators):
```cpp
// Single line with reusable propagator
auto extracted = W3C::TracingHelper::extractForTracer(trace_context);
auto extracted = B3::TracingHelper::extractForTracer(trace_context);
auto extracted = OpenTelemetry::TracingHelper::extractForTracer(trace_context);
```

### Enabling Multi-Format Support

**OpenTelemetry Tracer** (add B3 support):
```cpp
// Before: W3C only
if (hasW3CHeaders(trace_context)) {
  return extractW3CSpanContext(trace_context);
}

// After: W3C + B3 support
auto composite_context = OpenTelemetry::TracingHelper::extractForTracer(trace_context);
if (composite_context.has_value()) {
  return convertFromComposite(composite_context.value());
}
```

**Zipkin Tracer** (add W3C support):
```cpp
// Before: B3 only
auto b3_context = B3::TracingHelper::extractForTracer(trace_context);

// After: B3 + W3C support  
auto composite_context = OpenTelemetry::TracingHelper::extractForTracer(trace_context);
if (composite_context.has_value()) {
  return convertFromComposite(composite_context.value());
}
```

## Configuration Support

### OpenTelemetry Propagator Configuration

**Environment Variable** (takes precedence):
```bash
export OTEL_PROPAGATORS="tracecontext,b3,baggage"
```

**Proto Configuration**:
```yaml
# In OpenTelemetry tracer configuration
propagators:
  - "tracecontext"
  - "b3multi" 
  - "baggage"
```

**Supported Propagators**:
- `tracecontext` - W3C Trace Context format
- `baggage` - W3C Baggage format
- `b3` - B3 single header format
- `b3multi` - B3 multiple headers format  
- `none` - Disable all propagation

## Specification Compliance

All implementations provide complete compliance with official specifications:

### ✅ W3C Specifications
- [W3C Trace Context](https://www.w3.org/TR/trace-context/) - Complete traceparent/tracestate support
- [W3C Baggage](https://www.w3.org/TR/baggage/) - Full baggage specification with size limits

### ✅ B3 Specification  
- [B3 Propagation](https://github.com/openzipkin/b3-propagation) - Both single and multiple header formats
- [Zipkin Instrumentation](https://zipkin.io/pages/instrumenting.html) - Complete sampling state support

### ✅ OpenTelemetry Specification
- [OpenTelemetry Propagators](https://opentelemetry.io/docs/specs/otel/context/api-propagators/) - Composite propagator behavior
- [OpenTelemetry Configuration](https://opentelemetry.io/docs/languages/sdk-configuration/general/#otel_propagators) - OTEL_PROPAGATORS support

See [SPECIFICATION_COMPLIANCE.md](SPECIFICATION_COMPLIANCE.md) for detailed compliance information.

## Testing and Quality Assurance

### Comprehensive Test Coverage

- **3,274+ lines of test code** across all propagators
- **Specification compliance tests** validating against official requirements
- **Cross-format compatibility tests** ensuring proper conversion and interoperability
- **Error handling tests** covering malformed inputs and edge cases
- **Performance tests** validating efficient parsing and injection

See [TESTING_GUIDE.md](TESTING_GUIDE.md) for detailed testing strategies and examples.

### Validation Features

- **Round-trip consistency** - Extract/inject cycles maintain data integrity
- **Format conversion validation** - Proper handling of W3C ↔ B3 conversions
- **Error tolerance** - Graceful handling of malformed headers
- **Future compatibility** - Support for specification evolution

## Performance Characteristics

### Efficient Operations
- **Zero-copy design** where possible
- **Lazy conversion** - Format conversion only when needed
- **Minimal allocations** - Efficient string handling and validation
- **Fast header parsing** - Optimized for common case performance

### Memory Management
- **Bounded memory usage** - Size limits prevent excessive memory consumption
- **RAII patterns** - Automatic resource management
- **Move semantics** - Efficient transfer of large contexts

## Integration Examples

### Standard Span Baggage Interface

```cpp
class MySpan : public Tracing::Span {
public:
  std::string getBaggage(absl::string_view key) override {
    return W3C::BaggageHelper::getBaggageValue(trace_context_, key);
  }

  void setBaggage(absl::string_view key, absl::string_view value) override {
    W3C::BaggageHelper::setBaggageValue(trace_context_, key, value);
  }
};
```

### Cross-Format Service

```cpp
class MultiFormatService {
public:
  absl::Status handleRequest(const Tracing::TraceContext& incoming_context) {
    // Extract from any format (W3C or B3)
    auto composite_result = OpenTelemetry::Propagator::extract(incoming_context);
    if (composite_result.ok()) {
      auto context = composite_result.value();
      ENVOY_LOG(info, "Trace ID: {}, Format: {}", 
                context.getTraceId(), 
                context.format() == TraceFormat::W3C ? "W3C" : "B3");
    }
    return absl::OkStatus();
  }
};
```

## Contributing and Extending

### Adding New Propagators

To add support for additional propagation formats:

1. Create new directory under `source/extensions/propagators/`
2. Implement the core propagator interface
3. Add data structure definitions
4. Provide integration examples
5. Include comprehensive tests
6. Update composite propagator to include new format

### Extending Existing Propagators

For specification updates or new features:

1. Update the relevant propagator implementation
2. Add corresponding test coverage
3. Update documentation and examples
4. Ensure backward compatibility
5. Validate specification compliance

## Dependencies

### Build Dependencies
- `//envoy/tracing:trace_context_interface` - Envoy trace context interface
- `//source/common/tracing:trace_context_impl` - Trace context implementation
- Abseil libraries for status handling and utilities

### Runtime Dependencies  
- HTTP header manipulation capabilities
- Environment variable access (for OpenTelemetry configuration)
- Standard C++ libraries

## Future Roadmap

### Planned Enhancements
- **Additional Formats** - Support for Jaeger, AWS X-Ray propagation
- **Custom Propagators** - Plugin interface for organization-specific formats
- **Performance Optimization** - Header caching and batch operations
- **Metrics Integration** - Propagation success/failure metrics
- **Configuration Evolution** - Enhanced runtime configuration capabilities

### Specification Evolution
- **W3C Updates** - Support for future W3C specification versions
- **OpenTelemetry Evolution** - Updates to OpenTelemetry propagator specifications
- **B3 Extensions** - Support for B3 specification enhancements

This comprehensive propagator suite provides a robust foundation for distributed tracing in Envoy while maintaining the highest standards for specification compliance, performance, and maintainability.
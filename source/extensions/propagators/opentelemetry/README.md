# OpenTelemetry Composite Propagator

## Overview

The OpenTelemetry Composite Propagator provides a unified interface for handling multiple distributed tracing formats (W3C and B3) through a single API, following the [OpenTelemetry specification for composite propagators](https://opentelemetry.io/docs/specs/otel/context/api-propagators).

This implementation eliminates code duplication across Envoy tracers and provides comprehensive distributed tracing capabilities with automatic format detection and conversion.

## Architecture

### Core Components

#### 1. CompositeTraceContext (`trace_context.h/.cc`)

A unified trace context that can contain either W3C or B3 trace information:

```cpp
enum class TraceFormat {
  W3C,    // W3C Trace Context format
  B3,     // B3 Propagation format  
  NONE    // No valid trace context found
};

class CompositeTraceContext {
public:
  // Constructors for different formats
  explicit CompositeTraceContext(const W3C::TraceContext& w3c_context);
  explicit CompositeTraceContext(const B3::TraceContext& b3_context);
  
  // Format-agnostic accessors
  std::string getTraceId() const;
  std::string getSpanId() const;
  std::string getParentSpanId() const;
  bool isSampled() const;
  TraceFormat format() const;
  
  // Child span creation
  absl::StatusOr<CompositeTraceContext> createChild(absl::string_view new_span_id) const;
  
  // Format conversion
  absl::StatusOr<CompositeTraceContext> convertTo(TraceFormat target_format) const;
};
```

#### 2. CompositeBaggage

Unified baggage interface that currently supports W3C baggage (B3 doesn't have baggage):

```cpp
class CompositeBaggage {
public:
  std::string getValue(absl::string_view key) const;
  bool setValue(absl::string_view key, absl::string_view value);
  std::map<std::string, std::string> getAllEntries() const;
  bool isEmpty() const;
};
```

#### 3. Propagator (`propagator.h/.cc`)

Main composite propagator implementation with configurable behavior:

```cpp
class Propagator {
public:
  enum class InjectionFormat {
    W3C_ONLY,      // Inject only W3C headers
    B3_ONLY,       // Inject only B3 headers
    W3C_PRIMARY,   // Inject W3C with B3 fallback
    B3_PRIMARY,    // Inject B3 with W3C fallback
    BOTH           // Inject both formats
  };
  
  // Core extraction/injection
  static absl::StatusOr<CompositeTraceContext> extract(const Tracing::TraceContext& trace_context);
  static absl::Status inject(const CompositeTraceContext& composite_context, Tracing::TraceContext& trace_context);
  
  // Baggage operations
  static absl::StatusOr<CompositeBaggage> extractBaggage(const Tracing::TraceContext& trace_context);
  static absl::Status injectBaggage(const CompositeBaggage& baggage, Tracing::TraceContext& trace_context);
  
  // Context creation
  static absl::StatusOr<CompositeTraceContext> createRoot(absl::string_view trace_id, absl::string_view span_id, bool sampled, TraceFormat format = TraceFormat::W3C);
  static absl::StatusOr<CompositeTraceContext> createChild(const CompositeTraceContext& parent_context, absl::string_view new_span_id);
};
```

#### 4. TracingHelper

Backward compatibility interface for existing OpenTelemetry tracer integration:

```cpp
class TracingHelper {
public:
  // Direct replacement for existing SpanContextExtractor
  static absl::optional<CompositeTraceContext> extractForTracer(const Tracing::TraceContext& trace_context);
  static absl::Status injectFromTracer(const CompositeTraceContext& composite_context, Tracing::TraceContext& trace_context);
  
  // Compatibility methods
  static bool propagationHeaderPresent(const Tracing::TraceContext& trace_context);
  static absl::StatusOr<CompositeTraceContext> createFromTracerData(/* tracer-specific parameters */);
};
```

#### 5. BaggageHelper

Integration with standard Span baggage interface:

```cpp
class BaggageHelper {
public:
  // Direct replacement for tracer getBaggage()/setBaggage() methods
  static std::string getBaggageValue(const Tracing::TraceContext& trace_context, absl::string_view key);
  static bool setBaggageValue(Tracing::TraceContext& trace_context, absl::string_view key, absl::string_view value);
  static std::map<std::string, std::string> getAllBaggage(const Tracing::TraceContext& trace_context);
  static bool hasBaggage(const Tracing::TraceContext& trace_context);
};
```

## Usage Examples

### 1. Enhanced OpenTelemetry Tracer Integration

Replace existing OpenTelemetry tracer's `SpanContextExtractor` with composite propagator:

```cpp
// Before: OpenTelemetry tracer with W3C-only support
class OpenTelemetryTracer {
  absl::StatusOr<SpanContext> extractSpanContext(const Tracing::TraceContext& trace_context) {
    SpanContextExtractor extractor(trace_context);
    if (!extractor.propagationHeaderPresent()) {
      return absl::NotFoundError("No W3C headers found");
    }
    return extractor.extractSpanContext(); // Only W3C format
  }
  
  std::string getBaggage(absl::string_view key) override { 
    return ""; // Stub implementation
  }
};

// After: Enhanced tracer with multi-format support
class EnhancedOpenTelemetryTracer {
  absl::StatusOr<CompositeTraceContext> extractSpanContext(const Tracing::TraceContext& trace_context) {
    auto context = TracingHelper::extractForTracer(trace_context);
    if (!context.has_value()) {
      return absl::NotFoundError("No supported trace headers found");
    }
    return context.value(); // Supports W3C + B3 formats
  }
  
  std::string getBaggage(absl::string_view key) override {
    return BaggageHelper::getBaggageValue(trace_context_, key); // Full W3C Baggage support
  }
  
  void setBaggage(absl::string_view key, absl::string_view value) override {
    BaggageHelper::setBaggageValue(trace_context_, key, value);
  }
};
```

### 2. Multi-Format Service

Handle incoming requests with any supported format and propagate appropriately:

```cpp
class MultiFormatService {
public:
  absl::Status processRequest(const Tracing::TraceContext& incoming_context) {
    // Extract context from any supported format (W3C or B3)
    auto composite_result = Propagator::extract(incoming_context);
    if (!composite_result.ok()) {
      return composite_result.status();
    }
    
    auto composite_context = composite_result.value();
    
    ENVOY_LOG(info, "Processing trace: ID={}, Format={}, Sampled={}",
              composite_context.getTraceId(),
              formatToString(composite_context.format()),
              composite_context.isSampled());
    
    // Process baggage for user data
    auto baggage_result = Propagator::extractBaggage(incoming_context);
    if (baggage_result.ok() && !baggage_result.value().isEmpty()) {
      auto user_id = baggage_result.value().getValue("user.id");
      auto session_id = baggage_result.value().getValue("session.id");
      ENVOY_LOG(debug, "User context: user={}, session={}", user_id, session_id);
    }
    
    return absl::OkStatus();
  }
  
  absl::Status createOutgoingRequest(const CompositeTraceContext& parent_context,
                                   Tracing::TraceContext& outgoing_context,
                                   TraceFormat preferred_format = TraceFormat::W3C) {
    // Create child span
    std::string new_span_id = generateRandomSpanId();
    auto child_result = Propagator::createChild(parent_context, new_span_id);
    if (!child_result.ok()) {
      return child_result.status();
    }
    
    auto child_context = child_result.value();
    
    // Convert to preferred format if needed
    if (child_context.format() != preferred_format) {
      auto converted_result = child_context.convertTo(preferred_format);
      if (converted_result.ok()) {
        child_context = converted_result.value();
      }
    }
    
    // Inject in target format
    Propagator::Config config;
    config.injection_format = (preferred_format == TraceFormat::W3C) 
        ? Propagator::InjectionFormat::W3C_PRIMARY 
        : Propagator::InjectionFormat::B3_PRIMARY;
    config.enable_baggage = true;
    
    return Propagator::inject(child_context, outgoing_context, config);
  }
};
```

### 3. Legacy Tracer Migration

Gradual migration path for existing tracers:

```cpp
// Step 1: Drop-in replacement for existing extraction logic
class LegacyTracerAdapter {
public:
  struct ExtractedSpanContext {
    std::string trace_id;
    std::string span_id;
    bool sampled;
    TraceFormat format; // New: format information
  };
  
  absl::optional<ExtractedSpanContext> extractSpanContext(const Tracing::TraceContext& trace_context) {
    // Before: ~50 lines of W3C parsing code
    // After: Single call to composite propagator
    auto composite_result = TracingHelper::extractForTracer(trace_context);
    if (!composite_result.has_value()) {
      return absl::nullopt;
    }
    
    auto composite_context = composite_result.value();
    
    ExtractedSpanContext result;
    result.trace_id = composite_context.getTraceId();
    result.span_id = composite_context.getSpanId();
    result.sampled = composite_context.isSampled();
    result.format = composite_context.format(); // Now supports B3 too!
    
    return result;
  }
  
  // Step 2: Enhanced injection with multi-format support
  absl::Status injectSpanContext(absl::string_view trace_id, absl::string_view span_id, 
                                bool sampled, Tracing::TraceContext& trace_context,
                                bool include_b3_headers = false) {
    auto composite_result = TracingHelper::createFromTracerData(trace_id, span_id, "", sampled);
    if (!composite_result.ok()) {
      return composite_result.status();
    }
    
    Propagator::Config config;
    config.injection_format = include_b3_headers 
        ? Propagator::InjectionFormat::BOTH 
        : Propagator::InjectionFormat::W3C_ONLY;
    
    return Propagator::inject(composite_result.value(), trace_context, config);
  }
};
```

### 4. Advanced Distributed Context Operations

Comprehensive baggage and format conversion:

```cpp
class AdvancedDistributedContext {
public:
  absl::Status propagateUserSession(const Tracing::TraceContext& incoming_context,
                                  Tracing::TraceContext& outgoing_context,
                                  absl::string_view user_id, absl::string_view session_id) {
    // Extract existing context and baggage
    auto composite_result = Propagator::extract(incoming_context);
    if (!composite_result.ok()) {
      return composite_result.status();
    }
    
    auto baggage_result = Propagator::extractBaggage(incoming_context);
    CompositeBaggage baggage;
    if (baggage_result.ok()) {
      baggage = baggage_result.value();
    }
    
    // Add user session information
    baggage.setValue("user.id", user_id);
    baggage.setValue("session.id", session_id);
    baggage.setValue("service.name", "example-service");
    baggage.setValue("request.timestamp", getCurrentTimestamp());
    
    // Inject both trace context and baggage
    auto inject_status = Propagator::inject(composite_result.value(), outgoing_context);
    if (!inject_status.ok()) {
      return inject_status;
    }
    
    return Propagator::injectBaggage(baggage, outgoing_context);
  }
  
  absl::Status convertForDownstream(const Tracing::TraceContext& incoming_context,
                                  Tracing::TraceContext& outgoing_context,
                                  TraceFormat required_format) {
    auto composite_result = Propagator::extract(incoming_context);
    if (!composite_result.ok()) {
      return composite_result.status();
    }
    
    auto composite_context = composite_result.value();
    
    // Convert to required format
    if (composite_context.format() != required_format) {
      auto converted_result = composite_context.convertTo(required_format);
      if (!converted_result.ok()) {
        return converted_result.status();
      }
      composite_context = converted_result.value();
    }
    
    // Inject in target format
    Propagator::Config config;
    switch (required_format) {
      case TraceFormat::W3C:
        config.injection_format = Propagator::InjectionFormat::W3C_ONLY;
        break;
      case TraceFormat::B3:
        config.injection_format = Propagator::InjectionFormat::B3_ONLY;
        break;
      default:
        return absl::InvalidArgumentError("Unsupported target format");
    }
    
    return Propagator::inject(composite_context, outgoing_context, config);
  }
};
```

## Features and Benefits

### 1. **Unified Multi-Format Support**
- **W3C Trace Context**: Full specification compliance including traceparent, tracestate
- **B3 Propagation**: Both single header and multiple headers formats
- **Automatic Detection**: Tries W3C first, falls back to B3
- **Format Conversion**: Convert between W3C and B3 formats seamlessly

### 2. **Complete Baggage Support**
- **W3C Baggage**: Full specification compliance with size limits and validation
- **Integration**: Direct integration with existing `getBaggage()`/`setBaggage()` methods
- **User Data Propagation**: Session info, user IDs, feature flags, etc.

### 3. **Backward Compatibility**
- **Drop-in Replacement**: Existing tracer code requires minimal changes
- **TracingHelper**: Provides same interface as existing `SpanContextExtractor`
- **BaggageHelper**: Eliminates stub baggage implementations

### 4. **Performance and Efficiency**
- **Zero Copy**: Reuses existing W3C and B3 propagator implementations
- **Lazy Conversion**: Format conversion only when needed
- **Configurable Injection**: Choose specific formats to reduce header overhead

### 5. **Comprehensive Testing**
- **Format Compliance**: Validates W3C and B3 specification adherence
- **Round-trip Testing**: Ensures extraction/injection consistency
- **Error Handling**: Comprehensive validation and error reporting
- **Integration Testing**: Tests with real Envoy trace context

## Configuration Options

### Propagator Configuration

```cpp
Propagator::Config config;
config.injection_format = Propagator::InjectionFormat::W3C_PRIMARY; // Prefer W3C, fallback to B3
config.enable_baggage = true;                                      // Include baggage handling
config.strict_validation = false;                                  // Relaxed validation for compatibility
```

### TracingHelper Configuration

```cpp
TracingHelper::TracerConfig config;
config.preferred_format = TraceFormat::W3C;        // Prefer W3C format
config.enable_format_fallback = true;              // Enable B3 fallback
config.enable_baggage = true;                      // Enable baggage support
```

## Integration with Existing Tracers

The composite propagator can be integrated with existing Envoy tracers with minimal code changes:

### OpenTelemetry Tracer Integration

Replace the existing `SpanContextExtractor` usage:

```cpp
// Before
SpanContextExtractor extractor(trace_context);
if (extractor.propagationHeaderPresent()) {
  auto span_context = extractor.extractSpanContext();
  // Process W3C-only context
}

// After  
auto composite_context = TracingHelper::extractForTracer(trace_context);
if (composite_context.has_value()) {
  // Process W3C or B3 context
  convertFromComposite(composite_context.value());
}
```

### Other Tracers (Zipkin, Datadog, etc.)

Enable B3 support in non-Zipkin tracers:

```cpp
// Before: W3C-only
auto w3c_context = W3C::TracingHelper::extractForTracer(trace_context);

// After: W3C + B3 support
auto composite_context = OpenTelemetry::TracingHelper::extractForTracer(trace_context);
```

## Dependencies

The OpenTelemetry composite propagator depends on:

- `//source/extensions/propagators/w3c:propagator_lib` - W3C implementation
- `//source/extensions/propagators/b3:propagator_lib` - B3 implementation
- `//envoy/tracing:trace_context_interface` - Envoy trace context interface
- Standard Abseil libraries for status handling and optional types

## Future Enhancements

1. **Additional Formats**: Support for Jaeger, AWS X-Ray propagation formats
2. **Custom Propagators**: Plugin interface for custom propagation formats
3. **Performance Optimization**: Header caching and batch operations
4. **Metrics Integration**: Propagation success/failure metrics
5. **Configuration Driven**: Runtime configuration of propagation preferences

This implementation provides a solid foundation for unified distributed tracing in Envoy while maintaining backward compatibility and enabling future extensibility.
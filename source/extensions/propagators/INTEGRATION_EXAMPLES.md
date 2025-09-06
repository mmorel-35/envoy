# Propagator Integration Examples

This document provides comprehensive integration examples showing how to use propagators effectively in different Envoy tracer implementations and scenarios.

## Overview

Envoy's propagators are designed as reusable components that integrate seamlessly with all tracer implementations. This guide demonstrates common integration patterns, best practices, and real-world examples.

## Basic Integration Patterns

### Single Propagator Usage

The simplest integration uses one propagator for a specific protocol:

```cpp
#include "source/extensions/propagators/w3c/tracecontext/tracecontext_propagator.h"

class SimpleTracer {
private:
  TraceContextPropagator propagator_; // Class-level for performance

public:
  void extractContext(const Http::RequestHeaderMap& headers) {
    Tracing::HttpTraceContext trace_context(headers);
    
    if (propagator_.hasTraceParent(trace_context)) {
      auto traceparent = propagator_.extractTraceParent(trace_context);
      if (traceparent.has_value()) {
        auto parsed = propagator_.parseTraceParent(*traceparent);
        if (parsed.ok()) {
          setTraceId(parsed->trace_id);
          setParentSpanId(parsed->span_id);
          setSampled(parsed->sampled);
        }
      }
    }
  }
  
  void injectContext(Http::RequestHeaderMap& headers) {
    Tracing::HttpTraceContext trace_context(headers);
    propagator_.injectTraceParent(trace_context, "00", getCurrentTraceId(), 
                                  getCurrentSpanId(), isSampled());
  }
};
```

### Multi-Propagator Integration

For maximum compatibility, use multiple propagators with fallback logic:

```cpp
#include "source/extensions/propagators/w3c/tracecontext/tracecontext_propagator.h"
#include "source/extensions/propagators/b3/multi/b3_multi_propagator.h"
#include "source/extensions/propagators/w3c/baggage/baggage_propagator.h"

class MultiProtocolTracer {
private:
  TraceContextPropagator w3c_propagator_;
  B3MultiPropagator b3_propagator_;
  BaggagePropagator baggage_propagator_;

public:
  void extractContext(const Http::RequestHeaderMap& headers) {
    Tracing::HttpTraceContext trace_context(headers);
    
    // Try W3C TraceContext first (preferred)
    if (w3c_propagator_.hasTraceParent(trace_context)) {
      extractW3cContext(trace_context);
    } 
    // Fallback to B3 format
    else if (b3_propagator_.hasTraceId(trace_context)) {
      extractB3Context(trace_context);
    }
    
    // Always extract baggage if present
    if (baggage_propagator_.hasBaggage(trace_context)) {
      extractBaggageContext(trace_context);
    }
  }
  
  void injectContext(Http::RequestHeaderMap& headers) {
    Tracing::HttpTraceContext trace_context(headers);
    
    // Inject in both formats for compatibility
    w3c_propagator_.injectTraceParent(trace_context, "00", getCurrentTraceId(),
                                      getCurrentSpanId(), isSampled());
    
    b3_propagator_.injectTraceId(trace_context, getCurrentTraceId());
    b3_propagator_.injectSpanId(trace_context, getCurrentSpanId());
    b3_propagator_.injectSampled(trace_context, isSampled());
    
    // Inject correlation context
    if (hasBaggageData()) {
      baggage_propagator_.injectBaggage(trace_context, getBaggageMap());
    }
  }

private:
  void extractW3cContext(Tracing::TraceContext& context) {
    auto traceparent = w3c_propagator_.extractTraceParent(context);
    auto parsed = w3c_propagator_.parseTraceParent(*traceparent);
    
    setTraceId(parsed->trace_id);
    setParentSpanId(parsed->span_id);
    setSampled(parsed->sampled);
    
    // Extract tracestate if present
    auto tracestate = w3c_propagator_.extractTraceState(context);
    if (tracestate.has_value()) {
      setTraceState(*tracestate);
    }
  }
  
  void extractB3Context(Tracing::TraceContext& context) {
    auto trace_id = b3_propagator_.extractTraceId(context);
    auto span_id = b3_propagator_.extractSpanId(context);
    auto sampled = b3_propagator_.extractSampled(context);
    
    if (trace_id.has_value()) setTraceId(*trace_id);
    if (span_id.has_value()) setParentSpanId(*span_id);
    if (sampled.has_value()) setSampled(*sampled);
  }
  
  void extractBaggageContext(Tracing::TraceContext& context) {
    // Extract specific baggage values
    auto user_id = baggage_propagator_.getBaggageValue(context, "userId");
    auto session_id = baggage_propagator_.getBaggageValue(context, "sessionId");
    
    if (user_id.ok()) setUserId(*user_id);
    if (session_id.ok()) setSessionId(*session_id);
  }
};
```

## Envoy Tracer Integration Examples

### OpenTelemetry Tracer Integration

Real-world integration with OpenTelemetry tracer:

```cpp
class OpenTelemetryTracer : public Tracing::Tracer {
private:
  TraceContextPropagator trace_propagator_;
  BaggagePropagator baggage_propagator_;
  
public:
  // Implement Tracing::Tracer interface
  Tracing::SpanPtr startSpan(const Tracing::Config& config,
                            Tracing::TraceContext& trace_context,
                            const std::string& operation_name,
                            Tracing::Decision tracing_decision) override {
    
    // Extract existing context
    std::string trace_id, parent_span_id;
    bool sampled = (tracing_decision == Tracing::Decision::Traced);
    
    if (trace_propagator_.hasTraceParent(trace_context)) {
      auto traceparent = trace_propagator_.extractTraceParent(trace_context);
      if (traceparent.has_value()) {
        auto parsed = trace_propagator_.parseTraceParent(*traceparent);
        if (parsed.ok()) {
          trace_id = parsed->trace_id;
          parent_span_id = parsed->span_id;
          sampled = parsed->sampled;
        }
      }
    }
    
    // Create new span with extracted or generated context
    if (trace_id.empty()) {
      trace_id = generateTraceId();
    }
    
    std::string span_id = generateSpanId();
    
    // Create OpenTelemetry span
    auto span = std::make_unique<OpenTelemetrySpan>(
      operation_name, trace_id, span_id, parent_span_id, sampled,
      trace_propagator_, baggage_propagator_);
    
    return span;
  }
};

class OpenTelemetrySpan : public Tracing::Span {
private:
  TraceContextPropagator& trace_propagator_;
  BaggagePropagator& baggage_propagator_;
  std::string trace_id_;
  std::string span_id_;
  std::string parent_span_id_;
  bool sampled_;

public:
  void injectContext(Tracing::TraceContext& trace_context,
                    const Tracing::UpstreamContext&) override {
    // Inject W3C TraceContext
    trace_propagator_.injectTraceParent(trace_context, "00", trace_id_, 
                                        span_id_, sampled_);
    
    // Inject tracestate if we have vendor-specific data
    if (!vendor_state_.empty()) {
      trace_propagator_.injectTraceState(trace_context, vendor_state_);
    }
    
    // Propagate baggage
    if (!baggage_data_.empty()) {
      baggage_propagator_.injectBaggage(trace_context, baggage_data_);
    }
  }
  
  void setBaggage(const std::string& key, const std::string& value) override {
    // Store baggage for later injection
    BaggageMember member;
    member.key = key;
    member.value = value;
    baggage_data_[key] = member;
  }
  
  std::string getBaggage(const std::string& key) override {
    auto it = baggage_data_.find(key);
    return (it != baggage_data_.end()) ? it->second.value : "";
  }
};
```

### Zipkin Tracer Integration

Integration with Zipkin tracer supporting both B3 and W3C formats:

```cpp
class ZipkinTracer : public Tracing::Tracer {
private:
  B3MultiPropagator b3_propagator_;
  TraceContextPropagator w3c_propagator_;
  bool enable_w3c_fallback_;

public:
  ZipkinTracer(bool enable_w3c_fallback = true) 
    : enable_w3c_fallback_(enable_w3c_fallback) {}
  
  Tracing::SpanPtr startSpan(const Tracing::Config& config,
                            Tracing::TraceContext& trace_context,
                            const std::string& operation_name,
                            Tracing::Decision tracing_decision) override {
    
    std::string trace_id, parent_span_id;
    bool sampled = (tracing_decision == Tracing::Decision::Traced);
    
    // Extract B3 context first (primary format for Zipkin)
    if (b3_propagator_.hasTraceId(trace_context)) {
      auto b3_trace_id = b3_propagator_.extractTraceId(trace_context);
      auto b3_span_id = b3_propagator_.extractSpanId(trace_context);
      auto b3_sampled = b3_propagator_.extractSampled(trace_context);
      
      if (b3_trace_id.has_value()) trace_id = *b3_trace_id;
      if (b3_span_id.has_value()) parent_span_id = *b3_span_id;
      if (b3_sampled.has_value()) sampled = *b3_sampled;
    }
    // Fallback to W3C if enabled and B3 not present
    else if (enable_w3c_fallback_ && w3c_propagator_.hasTraceParent(trace_context)) {
      auto traceparent = w3c_propagator_.extractTraceParent(trace_context);
      if (traceparent.has_value()) {
        auto parsed = w3c_propagator_.parseTraceParent(*traceparent);
        if (parsed.ok()) {
          trace_id = parsed->trace_id;
          parent_span_id = parsed->span_id;
          sampled = parsed->sampled;
        }
      }
    }
    
    if (trace_id.empty()) {
      trace_id = generateTraceId();
    }
    
    return std::make_unique<ZipkinSpan>(
      operation_name, trace_id, generateSpanId(), parent_span_id, sampled,
      b3_propagator_, w3c_propagator_, enable_w3c_fallback_);
  }
};

class ZipkinSpan : public Tracing::Span {
private:
  B3MultiPropagator& b3_propagator_;
  TraceContextPropagator& w3c_propagator_;
  bool enable_w3c_fallback_;

public:
  void injectContext(Tracing::TraceContext& trace_context,
                    const Tracing::UpstreamContext&) override {
    // Always inject B3 format (primary for Zipkin)
    b3_propagator_.injectTraceId(trace_context, trace_id_);
    b3_propagator_.injectSpanId(trace_context, span_id_);
    b3_propagator_.injectSampled(trace_context, sampled_);
    
    if (!parent_span_id_.empty()) {
      b3_propagator_.injectParentSpanId(trace_context, parent_span_id_);
    }
    
    // Optionally inject W3C format for interoperability
    if (enable_w3c_fallback_) {
      w3c_propagator_.injectTraceParent(trace_context, "00", trace_id_,
                                        span_id_, sampled_);
    }
  }
};
```

### Fluentd Tracer Integration

Lightweight integration for Fluentd logging tracer:

```cpp
class FluentdTracer : public Tracing::Tracer {
private:
  TraceContextPropagator propagator_;

public:
  class SpanContextExtractor {
  private:
    TraceContextPropagator& propagator_;
    
  public:
    explicit SpanContextExtractor(TraceContextPropagator& propagator)
      : propagator_(propagator) {}
    
    absl::optional<std::string> extractTraceId(Tracing::TraceContext& trace_context) {
      if (!propagator_.hasTraceParent(trace_context)) {
        return absl::nullopt;
      }
      
      auto traceparent = propagator_.extractTraceParent(trace_context);
      if (!traceparent.has_value()) {
        return absl::nullopt;
      }
      
      auto parsed = propagator_.parseTraceParent(*traceparent);
      return parsed.ok() ? absl::make_optional(parsed->trace_id) : absl::nullopt;
    }
    
    absl::optional<std::string> extractSpanId(Tracing::TraceContext& trace_context) {
      if (!propagator_.hasTraceParent(trace_context)) {
        return absl::nullopt;
      }
      
      auto traceparent = propagator_.extractTraceParent(trace_context);
      if (!traceparent.has_value()) {
        return absl::nullopt;
      }
      
      auto parsed = propagator_.parseTraceParent(*traceparent);
      return parsed.ok() ? absl::make_optional(parsed->span_id) : absl::nullopt;
    }
  };
  
  SpanContextExtractor getExtractor() {
    return SpanContextExtractor(propagator_);
  }
};
```

## Advanced Integration Patterns

### Composite Propagator Pattern

For complex scenarios requiring multiple propagation strategies:

```cpp
class CompositePropagator {
private:
  std::vector<std::unique_ptr<PropagatorInterface>> propagators_;
  
public:
  void addPropagator(std::unique_ptr<PropagatorInterface> propagator) {
    propagators_.push_back(std::move(propagator));
  }
  
  bool extractContext(Tracing::TraceContext& trace_context, TraceData& data) {
    for (auto& propagator : propagators_) {
      if (propagator->canExtract(trace_context)) {
        return propagator->extractContext(trace_context, data);
      }
    }
    return false;
  }
  
  void injectContext(Tracing::TraceContext& trace_context, const TraceData& data) {
    // Inject using all propagators for maximum compatibility
    for (auto& propagator : propagators_) {
      propagator->injectContext(trace_context, data);
    }
  }
};

// Usage
CompositePropagator composite;
composite.addPropagator(std::make_unique<W3cTraceContextAdapter>());
composite.addPropagator(std::make_unique<B3MultiAdapter>());
composite.addPropagator(std::make_unique<XRayAdapter>());
```

### Configuration-Driven Integration

Dynamic propagator selection based on configuration:

```cpp
class ConfigurableTracer {
private:
  std::unique_ptr<PropagatorInterface> primary_propagator_;
  std::vector<std::unique_ptr<PropagatorInterface>> fallback_propagators_;
  
public:
  void configure(const TracingConfig& config) {
    // Create primary propagator based on config
    switch (config.primary_format()) {
    case TracingFormat::W3C_TRACE_CONTEXT:
      primary_propagator_ = std::make_unique<W3cTraceContextAdapter>();
      break;
    case TracingFormat::B3_MULTI:
      primary_propagator_ = std::make_unique<B3MultiAdapter>();
      break;
    case TracingFormat::XRAY:
      primary_propagator_ = std::make_unique<XRayAdapter>();
      break;
    }
    
    // Add fallback propagators
    for (auto format : config.fallback_formats()) {
      fallback_propagators_.push_back(createPropagator(format));
    }
  }
  
  void extractContext(Tracing::TraceContext& trace_context) {
    // Try primary first
    if (primary_propagator_->canExtract(trace_context)) {
      primary_propagator_->extractContext(trace_context, trace_data_);
      return;
    }
    
    // Try fallbacks
    for (auto& propagator : fallback_propagators_) {
      if (propagator->canExtract(trace_context)) {
        propagator->extractContext(trace_context, trace_data_);
        return;
      }
    }
  }
};
```

## Performance Optimization Patterns

### Lazy Propagator Initialization

```cpp
class LazyPropagatorTracer {
private:
  mutable std::unique_ptr<TraceContextPropagator> trace_propagator_;
  mutable std::unique_ptr<BaggagePropagator> baggage_propagator_;
  mutable std::once_flag init_flag_;
  
  void initializePropagators() const {
    std::call_once(init_flag_, [this]() {
      trace_propagator_ = std::make_unique<TraceContextPropagator>();
      baggage_propagator_ = std::make_unique<BaggagePropagator>();
    });
  }
  
public:
  void extractContext(Tracing::TraceContext& trace_context) {
    initializePropagators();
    
    if (trace_propagator_->hasTraceParent(trace_context)) {
      // Extract using initialized propagator
    }
  }
};
```

### Cached Validation Pattern

```cpp
class CachedValidationTracer {
private:
  TraceContextPropagator propagator_;
  mutable std::unordered_map<std::string, bool> validation_cache_;
  
public:
  bool isValidTraceparent(const std::string& traceparent) const {
    auto it = validation_cache_.find(traceparent);
    if (it != validation_cache_.end()) {
      return it->second;
    }
    
    auto result = propagator_.parseTraceParent(traceparent);
    bool valid = result.ok();
    validation_cache_[traceparent] = valid;
    return valid;
  }
};
```

## Error Handling Patterns

### Robust Error Handling

```cpp
class RobustTracer {
private:
  TraceContextPropagator propagator_;
  
  void logPropagationError(const std::string& operation, 
                          const absl::Status& status) {
    ENVOY_LOG(debug, "Propagation error in {}: {}", operation, status.message());
  }
  
public:
  void extractContext(Tracing::TraceContext& trace_context) {
    try {
      if (!propagator_.hasTraceParent(trace_context)) {
        return; // No trace context present
      }
      
      auto traceparent = propagator_.extractTraceParent(trace_context);
      if (!traceparent.has_value()) {
        ENVOY_LOG(debug, "Failed to extract traceparent header");
        return;
      }
      
      auto parsed = propagator_.parseTraceParent(*traceparent);
      if (!parsed.ok()) {
        logPropagationError("parseTraceParent", parsed.status());
        return;
      }
      
      // Use parsed context
      setTraceId(parsed->trace_id);
      setParentSpanId(parsed->span_id);
      setSampled(parsed->sampled);
      
    } catch (const std::exception& e) {
      ENVOY_LOG(error, "Exception during trace context extraction: {}", e.what());
    }
  }
};
```

## Testing Integration Examples

### Mock Propagator for Testing

```cpp
class MockPropagatorTracer {
private:
  std::unique_ptr<PropagatorInterface> propagator_;
  
public:
  explicit MockPropagatorTracer(std::unique_ptr<PropagatorInterface> propagator)
    : propagator_(std::move(propagator)) {}
  
  // Enable dependency injection for testing
  void setPropagator(std::unique_ptr<PropagatorInterface> propagator) {
    propagator_ = std::move(propagator);
  }
};

// Test usage
TEST(TracerTest, PropagationHandling) {
  auto mock_propagator = std::make_unique<MockPropagator>();
  EXPECT_CALL(*mock_propagator, extractTraceParent(_))
    .WillOnce(Return(absl::make_optional("00-trace123-span456-01")));
  
  MockPropagatorTracer tracer(std::move(mock_propagator));
  // Test tracer behavior...
}
```

This integration guide provides comprehensive examples for implementing propagators in various Envoy tracer scenarios, ensuring optimal performance and robustness.

#pragma once

/**
 * OpenTelemetry Composite Propagator Integration Examples
 * 
 * This file demonstrates how to integrate the OpenTelemetry composite propagator
 * with existing Envoy tracers and new applications.
 * 
 * The composite propagator provides a unified interface for handling multiple
 * trace formats (W3C and B3) through a single API, eliminating the need for
 * duplicate trace parsing logic across tracers.
 */

#include "source/extensions/propagators/opentelemetry/propagator.h"
#include "envoy/tracing/tracer.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace OpenTelemetry {
namespace Examples {

/**
 * Example: Enhanced OpenTelemetry Tracer Integration
 * 
 * Shows how to replace existing OpenTelemetry tracer's span context extraction
 * with the composite propagator to support both W3C and B3 formats.
 */
class EnhancedOpenTelemetryTracer {
public:
  /**
   * Enhanced span context extraction using composite propagator.
   * Replaces the existing SpanContextExtractor with multi-format support.
   */
  absl::StatusOr<CompositeTraceContext> extractSpanContext(const Tracing::TraceContext& trace_context) {
    // Configure for OpenTelemetry preferences
    TracingHelper::TracerConfig config;
    config.preferred_format = TraceFormat::W3C;      // Prefer W3C as primary
    config.enable_format_fallback = true;           // Enable B3 fallback
    config.enable_baggage = true;                   // Enable baggage support
    
    auto context = TracingHelper::extractForTracer(trace_context, config);
    if (!context.has_value()) {
      return absl::NotFoundError("No supported trace headers found");
    }
    
    return context.value();
  }

  /**
   * Enhanced span context injection with multi-format support.
   */
  absl::Status injectSpanContext(const CompositeTraceContext& composite_context,
                                Tracing::TraceContext& trace_context) {
    // Configure injection preferences
    TracingHelper::TracerConfig config;
    config.preferred_format = TraceFormat::W3C;      // Inject W3C primarily
    config.enable_baggage = true;                   // Include baggage
    
    return TracingHelper::injectFromTracer(composite_context, trace_context, config);
  }

  /**
   * Backward compatible header presence check.
   * Replaces existing propagationHeaderPresent() method.
   */
  bool propagationHeaderPresent(const Tracing::TraceContext& trace_context) {
    return TracingHelper::propagationHeaderPresent(trace_context);
  }

  /**
   * Enhanced baggage support using composite propagator.
   * Replaces stub getBaggage() implementations.
   */
  std::string getBaggage(const Tracing::TraceContext& trace_context, absl::string_view key) {
    return BaggageHelper::getBaggageValue(trace_context, key);
  }

  /**
   * Enhanced baggage setting using composite propagator.
   * Replaces stub setBaggage() implementations.
   */
  bool setBaggage(Tracing::TraceContext& trace_context, 
                 absl::string_view key, absl::string_view value) {
    return BaggageHelper::setBaggageValue(trace_context, key, value);
  }
};

/**
 * Example: Multi-Format Distributed Tracing Service
 * 
 * Shows how to build a service that can handle incoming requests with either
 * W3C or B3 headers and propagate context in the appropriate format.
 */
class MultiFormatTracingService {
public:
  /**
   * Process incoming request with automatic format detection.
   */
  absl::Status processIncomingRequest(const Tracing::TraceContext& incoming_context) {
    // Extract context from any supported format
    auto composite_result = Propagator::extract(incoming_context);
    if (!composite_result.ok()) {
      return composite_result.status();
    }
    
    auto composite_context = composite_result.value();
    
    // Log trace information regardless of format
    ENVOY_LOG(info, "Processing trace: ID={}, Span={}, Format={}, Sampled={}",
              composite_context.getTraceId(),
              composite_context.getSpanId(),
              formatToString(composite_context.format()),
              composite_context.isSampled());
    
    // Extract and process baggage if available
    auto baggage_result = Propagator::extractBaggage(incoming_context);
    if (baggage_result.ok() && !baggage_result.value().isEmpty()) {
      processBaggage(baggage_result.value());
    }
    
    return absl::OkStatus();
  }

  /**
   * Create outgoing request with preferred format.
   */
  absl::Status createOutgoingRequest(const CompositeTraceContext& parent_context,
                                   Tracing::TraceContext& outgoing_context,
                                   TraceFormat preferred_format = TraceFormat::W3C) {
    // Generate new span ID for child
    std::string new_span_id = generateRandomSpanId();
    
    // Create child context
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
    
    // Configure injection
    Propagator::Config config;
    if (preferred_format == TraceFormat::W3C) {
      config.propagators = {PropagatorType::TraceContext, PropagatorType::B3};
    } else {
      config.propagators = {PropagatorType::B3, PropagatorType::TraceContext};
    }
    config.enable_baggage = true;
    
    return Propagator::inject(child_context, outgoing_context, config);
  }

private:
  std::string formatToString(TraceFormat format) {
    switch (format) {
      case TraceFormat::W3C: return "W3C";
      case TraceFormat::B3: return "B3";
      case TraceFormat::NONE: return "NONE";
      default: return "UNKNOWN";
    }
  }

  void processBaggage(const CompositeBaggage& baggage) {
    auto entries = baggage.getAllEntries();
    for (const auto& entry : entries) {
      ENVOY_LOG(debug, "Baggage: {}={}", entry.first, entry.second);
    }
  }

  std::string generateRandomSpanId() {
    // Implementation would generate a random 16-character hex string
    return "0123456789abcdef";
  }
};

/**
 * Example: Legacy Tracer Migration
 * 
 * Shows how to gradually migrate existing tracers from hardcoded format
 * support to the composite propagator.
 */
class LegacyTracerAdapter {
public:
  /**
   * Drop-in replacement for existing W3C-only extraction logic.
   * Before: ~50 lines of W3C parsing code in each tracer
   * After: Single call to composite propagator
   */
  struct ExtractedSpanContext {
    std::string trace_id;
    std::string span_id;
    std::string parent_span_id;
    bool sampled;
    std::string trace_state;
    TraceFormat format;
  };

  absl::optional<ExtractedSpanContext> extractSpanContext(const Tracing::TraceContext& trace_context) {
    auto composite_result = TracingHelper::extractForTracer(trace_context);
    if (!composite_result.has_value()) {
      return absl::nullopt;
    }
    
    auto composite_context = composite_result.value();
    
    ExtractedSpanContext result;
    result.trace_id = composite_context.getTraceId();
    result.span_id = composite_context.getSpanId();
    result.parent_span_id = composite_context.getParentSpanId();
    result.sampled = composite_context.isSampled();
    result.trace_state = composite_context.getTraceState();
    result.format = composite_context.format();
    
    return result;
  }

  /**
   * Enhanced injection that can output multiple formats.
   * Before: Only W3C format supported
   * After: W3C + B3 support with automatic conversion
   */
  absl::Status injectSpanContext(absl::string_view trace_id,
                                absl::string_view span_id,
                                bool sampled,
                                Tracing::TraceContext& trace_context,
                                bool include_b3_headers = false) {
    // Create W3C context (maintains backward compatibility)
    auto composite_result = TracingHelper::createFromTracerData(
        trace_id, span_id, "", sampled, "", TraceFormat::W3C);
    if (!composite_result.ok()) {
      return composite_result.status();
    }
    
    // Configure injection format
    Propagator::Config config;
    if (include_b3_headers) {
      config.propagators = {PropagatorType::TraceContext, PropagatorType::B3, PropagatorType::Baggage};
    } else {
      config.propagators = {PropagatorType::TraceContext, PropagatorType::Baggage};
    }
    
    return Propagator::inject(composite_result.value(), trace_context, config);
  }
};

/**
 * Example: Advanced Distributed Context Operations
 * 
 * Shows advanced use cases for distributed tracing with baggage and
 * cross-format operations.
 */
class AdvancedDistributedContext {
public:
  /**
   * Propagate user session data across service boundaries.
   */
  absl::Status propagateUserSession(const Tracing::TraceContext& incoming_context,
                                  Tracing::TraceContext& outgoing_context,
                                  absl::string_view user_id,
                                  absl::string_view session_id) {
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
    
    // Add user session information to baggage
    baggage.setValue("user.id", user_id);
    baggage.setValue("session.id", session_id);
    baggage.setValue("service.name", "example-service");
    baggage.setValue("request.timestamp", getCurrentTimestamp());
    
    // Inject trace context and baggage
    auto inject_status = Propagator::inject(composite_result.value(), outgoing_context);
    if (!inject_status.ok()) {
      return inject_status.status();
    }
    
    return Propagator::injectBaggage(baggage, outgoing_context);
  }

  /**
   * Format conversion for downstream service compatibility.
   */
  absl::Status convertForDownstreamService(const Tracing::TraceContext& incoming_context,
                                         Tracing::TraceContext& outgoing_context,
                                         TraceFormat required_format) {
    // Extract in any format
    auto composite_result = Propagator::extract(incoming_context);
    if (!composite_result.ok()) {
      return composite_result.status();
    }
    
    auto composite_context = composite_result.value();
    
    // Convert to required format if needed
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
        config.propagators = {PropagatorType::TraceContext};
        break;
      case TraceFormat::B3:
        config.propagators = {PropagatorType::B3};
        break;
      default:
        return absl::InvalidArgumentError("Unsupported target format");
    }
    
    return Propagator::inject(composite_context, outgoing_context, config);
  }

private:
  std::string getCurrentTimestamp() {
    // Implementation would return current ISO 8601 timestamp
    // Example: Use Envoy's time system for accurate timestamps
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
  }
};

} // namespace Examples
} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
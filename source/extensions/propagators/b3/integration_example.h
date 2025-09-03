#pragma once

/**
 * @file integration_example.h
 * 
 * This file provides comprehensive examples of using the B3 propagator
 * in real-world scenarios for both new and existing tracer integrations.
 */

#include "source/extensions/propagators/b3/propagator.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace B3 {
namespace Examples {

/**
 * Example: Enhanced Zipkin tracer integration using B3 propagator
 * 
 * This shows how the existing Zipkin tracer can be simplified by using
 * the reusable B3 propagator instead of maintaining its own parsing logic.
 */
class EnhancedZipkinTracer {
public:
  // Before: ~200 lines of B3 parsing logic in span_context_extractor.cc
  // After: Clean integration with B3 propagator

  /**
   * Extract span context from B3 headers with full format support.
   */
  absl::StatusOr<ZipkinSpanContext> extractSpanContext(const Tracing::TraceContext& trace_context) {
    // Check if B3 headers are present
    if (!B3::Propagator::isPresent(trace_context)) {
      return absl::NotFoundError("No B3 headers found");
    }

    // Extract B3 context using the propagator
    auto b3_result = B3::TracingHelper::extractForTracer(trace_context);
    if (!b3_result.has_value()) {
      return absl::InvalidArgumentError("Failed to extract B3 context");
    }

    const auto& b3_context = b3_result.value();
    
    // Convert to Zipkin's internal format
    return ZipkinSpanContext(
        b3_context.traceId().high(),     // trace_id_high
        b3_context.traceId().low(),      // trace_id_low  
        b3_context.spanId().value(),     // span_id
        b3_context.parentSpanId().has_value() ? 
            b3_context.parentSpanId().value().value() : 0, // parent_span_id
        B3::TracingHelper::isSampled(b3_context.samplingState()) // sampled
    );
  }

  /**
   * Inject span context to B3 headers.
   */
  absl::Status injectSpanContext(const ZipkinSpanContext& span_context,
                                Tracing::TraceContext& trace_context) {
    // Create B3 context from Zipkin data
    auto b3_context = B3::TracingHelper::createTraceContext(
        span_context.trace_id_high(),
        span_context.trace_id_low(),
        span_context.span_id(),
        span_context.parent_span_id(),
        span_context.sampled()
    );

    // Inject using B3 propagator
    return B3::TracingHelper::injectFromTracer(b3_context, trace_context);
  }
};

/**
 * Example: Adding B3 support to OpenTelemetry tracer
 * 
 * This demonstrates how other tracers can easily add B3 support
 * using the reusable propagator.
 */
class OpenTelemetryWithB3Support {
public:
  /**
   * Extract trace context supporting both W3C and B3 formats.
   */
  absl::StatusOr<OtelSpanContext> extractSpanContext(const Tracing::TraceContext& trace_context) {
    // Try W3C format first (existing logic)
    if (hasW3CHeaders(trace_context)) {
      return extractW3CSpanContext(trace_context);
    }

    // Try B3 format as fallback
    if (B3::Propagator::isPresent(trace_context)) {
      auto b3_result = B3::TracingHelper::extractForTracer(trace_context);
      if (b3_result.has_value()) {
        return convertB3ToOtel(b3_result.value());
      }
    }

    return absl::NotFoundError("No supported trace headers found");
  }

private:
  /**
   * Convert B3 context to OpenTelemetry format.
   */
  OtelSpanContext convertB3ToOtel(const B3::TraceContext& b3_context) {
    // Convert B3 128-bit trace ID to OTel format
    std::string trace_id = b3_context.traceId().toHexString();
    if (trace_id.length() == 16) {
      // Pad 64-bit trace ID to 128-bit for OTel
      trace_id = std::string(16, '0') + trace_id;
    }

    return OtelSpanContext(
        trace_id,                                    // 32-char hex trace ID
        b3_context.spanId().toHexString(),          // 16-char hex span ID  
        B3::TracingHelper::isSampled(b3_context.samplingState()) // sampled
    );
  }
};

/**
 * Example: Universal trace context handler
 * 
 * This shows how to create a service that can handle multiple
 * trace propagation formats simultaneously.
 */
class UniversalTraceHandler {
public:
  struct TraceInfo {
    std::string trace_id;
    std::string span_id;
    std::string parent_span_id;
    bool sampled;
    std::string format; // "w3c", "b3", etc.
  };

  /**
   * Extract trace information from any supported format.
   */
  absl::optional<TraceInfo> extractTraceInfo(const Tracing::TraceContext& trace_context) {
    // Try B3 format
    if (B3::Propagator::isPresent(trace_context)) {
      auto b3_result = B3::Propagator::extract(trace_context);
      if (b3_result.ok()) {
        const auto& context = b3_result.value();
        return TraceInfo{
            context.traceId().toHexString(),
            context.spanId().toHexString(),
            context.parentSpanId().has_value() ? 
                context.parentSpanId().value().toHexString() : "",
            context.sampled(),
            "b3"
        };
      }
    }

    // Try W3C format
    // ... (W3C extraction logic)

    return absl::nullopt;
  }

  /**
   * Inject trace information in specified format.
   */
  absl::Status injectTraceInfo(const TraceInfo& info, 
                              const std::string& format,
                              Tracing::TraceContext& trace_context) {
    if (format == "b3") {
      // Create B3 context from trace info
      auto trace_id = B3::TraceId::fromHexString(info.trace_id);
      auto span_id = B3::SpanId::fromHexString(info.span_id);
      
      if (!trace_id.ok() || !span_id.ok()) {
        return absl::InvalidArgumentError("Invalid trace information");
      }

      absl::optional<B3::SpanId> parent_span_id;
      if (!info.parent_span_id.empty()) {
        auto parent_result = B3::SpanId::fromHexString(info.parent_span_id);
        if (parent_result.ok()) {
          parent_span_id = parent_result.value();
        }
      }

      B3::SamplingState sampling_state = info.sampled ? 
          B3::SamplingState::SAMPLED : 
          B3::SamplingState::NOT_SAMPLED;

      B3::TraceContext b3_context(trace_id.value(), span_id.value(), 
                                  parent_span_id, sampling_state);
      
      return B3::Propagator::inject(b3_context, trace_context);
    }

    return absl::InvalidArgumentError("Unsupported format");
  }
};

/**
 * Example: B3 format conversion utilities
 * 
 * This demonstrates advanced B3 operations like format conversion
 * and header optimization.
 */
class B3FormatConverter {
public:
  /**
   * Convert B3 multiple headers to single header format.
   */
  absl::StatusOr<std::string> convertToSingleHeader(const Tracing::TraceContext& trace_context) {
    auto b3_context = B3::Propagator::extractMultipleHeaders(trace_context);
    if (!b3_context.ok()) {
      return b3_context.status();
    }

    return b3_context.value().toSingleHeader();
  }

  /**
   * Convert B3 single header to multiple headers format.
   */
  absl::Status convertToMultipleHeaders(const std::string& single_header,
                                       Tracing::TraceContext& trace_context) {
    // Create temporary trace context with single header
    Tracing::TraceContextImpl temp_context;
    temp_context.setByKey("b3", single_header);

    // Extract and re-inject as multiple headers
    auto b3_context = B3::Propagator::extractSingleHeader(temp_context);
    if (!b3_context.ok()) {
      return b3_context.status();
    }

    return B3::Propagator::injectMultipleHeaders(b3_context.value(), trace_context);
  }

  /**
   * Optimize B3 headers by choosing the most compact format.
   */
  absl::Status optimizeB3Headers(Tracing::TraceContext& trace_context) {
    auto b3_context = B3::Propagator::extract(trace_context);
    if (!b3_context.ok()) {
      return b3_context.status();
    }

    // Clear existing B3 headers
    clearB3Headers(trace_context);

    // Calculate sizes for both formats
    auto single_header = b3_context.value().toSingleHeader();
    if (!single_header.ok()) {
      return single_header.status();
    }

    const size_t single_header_size = single_header.value().size();
    const size_t multiple_headers_size = calculateMultipleHeadersSize(b3_context.value());

    // Use the more compact format
    if (single_header_size <= multiple_headers_size) {
      return B3::Propagator::injectSingleHeader(b3_context.value(), trace_context);
    } else {
      return B3::Propagator::injectMultipleHeaders(b3_context.value(), trace_context);
    }
  }

private:
  void clearB3Headers(Tracing::TraceContext& trace_context) {
    // Implementation would clear existing B3 headers
  }

  size_t calculateMultipleHeadersSize(const B3::TraceContext& context) {
    // Implementation would calculate total size of multiple headers
    return 0; // Placeholder
  }
};

} // namespace Examples
} // namespace B3
} // namespace Propagators  
} // namespace Extensions
} // namespace Envoy
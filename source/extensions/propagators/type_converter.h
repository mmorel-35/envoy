#pragma once

#include "source/extensions/propagators/trace_context.h"
#include "source/extensions/tracers/opentelemetry/span_context.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {

/**
 * Utility functions to convert between generic propagator types and tracer-specific types.
 * This allows tracers to maintain their existing APIs while using shared propagators.
 */
class TypeConverter {
public:
  /**
   * Convert generic SpanContext to OpenTelemetry SpanContext.
   * Note: Parent span ID is not included in OpenTelemetry SpanContext as it doesn't support it.
   */
  static Tracers::OpenTelemetry::SpanContext 
  toOpenTelemetrySpanContext(const SpanContext& generic_context);

  /**
   * Convert OpenTelemetry SpanContext to generic SpanContext.
   */
  static SpanContext 
  fromOpenTelemetrySpanContext(const Tracers::OpenTelemetry::SpanContext& otel_context);

  /**
   * Extract parent span ID from generic SpanContext.
   * This is useful for tracers like Zipkin that need parent span ID information.
   */
  static absl::optional<std::string> 
  extractParentSpanId(const SpanContext& generic_context);

  /**
   * Create a generic SpanContext with parent span ID information.
   * This is useful when converting from tracers that support parent span IDs.
   */
  static SpanContext 
  createSpanContextWithParent(const TraceId& trace_id, const SpanId& span_id, 
                              const TraceFlags& trace_flags, const SpanId& parent_span_id,
                              const std::string& tracestate = "");

  /**
   * Convert trace ID string to generic TraceId.
   * Handles various trace ID formats (16-byte, 32-byte hex strings).
   */
  static TraceId toTraceId(absl::string_view trace_id_str);

  /**
   * Convert span ID string to generic SpanId.
   * Handles 8-byte hex strings.
   */
  static SpanId toSpanId(absl::string_view span_id_str);

  /**
   * Convert sampling boolean to generic TraceFlags.
   */
  static TraceFlags toTraceFlags(bool sampled);

  /**
   * Parse B3 sampling state to boolean.
   * Handles "0", "1", "d" values according to B3 specification.
   */
  static absl::optional<bool> parseB3SamplingState(absl::string_view sampling_state);
};

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
#pragma once

#include "envoy/common/exception.h"
#include "envoy/tracing/tracer.h"

#include "source/common/common/statusor.h"
#include "source/common/http/header_map_impl.h"
#include "source/common/tracing/trace_context_impl.h"
#include "source/extensions/propagators/opentelemetry/propagator.h"
#include "source/extensions/tracers/opentelemetry/span_context.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

/**
 * This class is used to extract SpanContext from HTTP headers.
 * Refactored to use the new OpenTelemetry composite propagator for
 * multi-format support (W3C + B3) and improved specification compliance.
 */
class SpanContextExtractor {
public:
  SpanContextExtractor(Tracing::TraceContext& trace_context);
  ~SpanContextExtractor();
  absl::StatusOr<SpanContext> extractSpanContext();
  bool propagationHeaderPresent();

private:
  /**
   * Convert composite trace context to OpenTelemetry span context format
   */
  absl::StatusOr<SpanContext> convertCompositeToOtel(
      const Extensions::Propagators::OpenTelemetry::CompositeTraceContext& composite_context);

  const Tracing::TraceContext& trace_context_;
};

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

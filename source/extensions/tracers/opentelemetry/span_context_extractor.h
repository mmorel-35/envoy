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
 * Updated to directly use the OpenTelemetry composite propagator.
 */
class SpanContextExtractor {
public:
  SpanContextExtractor(Tracing::TraceContext& trace_context, 
                       const Extensions::Propagators::OpenTelemetry::Propagator::Config& propagator_config);
  ~SpanContextExtractor();
  absl::StatusOr<SpanContext> extractSpanContext();
  bool propagationHeaderPresent();

private:
  /**
   * Convert CompositeTraceContext to OpenTelemetry SpanContext
   */
  absl::StatusOr<SpanContext> convertFromComposite(
      const Extensions::Propagators::OpenTelemetry::CompositeTraceContext& composite_context);

  const Tracing::TraceContext& trace_context_;
  const Extensions::Propagators::OpenTelemetry::Propagator::Config& propagator_config_;
};

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

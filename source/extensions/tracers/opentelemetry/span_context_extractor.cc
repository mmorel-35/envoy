#include "source/extensions/tracers/opentelemetry/span_context_extractor.h"

#include "envoy/tracing/tracer.h"

#include "source/common/http/header_map_impl.h"
#include "source/common/tracing/trace_context_impl.h"
#include "source/extensions/propagators/opentelemetry/propagator.h"
#include "source/extensions/tracers/opentelemetry/span_context.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

SpanContextExtractor::SpanContextExtractor(Tracing::TraceContext& trace_context,
                                          const Extensions::Propagators::OpenTelemetry::PropagatorService& propagator_service)
    : trace_context_(trace_context), propagator_service_(propagator_service) {}

SpanContextExtractor::~SpanContextExtractor() = default;

bool SpanContextExtractor::propagationHeaderPresent() {
  return propagator_service_.isPresent(trace_context_);
}

absl::StatusOr<SpanContext> SpanContextExtractor::extractSpanContext() {
  auto composite_result = propagator_service_.extract(trace_context_);
  if (!composite_result.ok()) {
    return composite_result.status();
  }
  
  return convertFromComposite(composite_result.value());
}

absl::StatusOr<SpanContext> SpanContextExtractor::convertFromComposite(
    const Extensions::Propagators::OpenTelemetry::CompositeTraceContext& composite_context) {
  
  // Create OpenTelemetry SpanContext from composite context
  SpanContext span_context(
    "00",                                    // version (default for compatibility)
    composite_context.getTraceId(),         // trace_id
    composite_context.getSpanId(),          // span_id
    composite_context.isSampled(),          // sampled
    composite_context.getTraceState()       // trace_state
  );
  
  return span_context;
}

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

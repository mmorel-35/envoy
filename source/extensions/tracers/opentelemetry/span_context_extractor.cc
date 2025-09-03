#include "source/extensions/tracers/opentelemetry/span_context_extractor.h"

#include "envoy/tracing/tracer.h"

#include "source/common/http/header_map_impl.h"
#include "source/common/tracing/trace_context_impl.h"
#include "source/extensions/tracers/opentelemetry/propagator_config.h"
#include "source/extensions/tracers/opentelemetry/span_context.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

SpanContextExtractor::SpanContextExtractor(Tracing::TraceContext& trace_context,
                                          const PropagatorConfig& propagator_config)
    : trace_context_(trace_context), propagator_config_(propagator_config) {}

SpanContextExtractor::~SpanContextExtractor() = default;

bool SpanContextExtractor::propagationHeaderPresent() {
  return propagator_config_.propagationHeaderPresent(trace_context_);
}

absl::StatusOr<SpanContext> SpanContextExtractor::extractSpanContext() {
  return propagator_config_.extractSpanContext(trace_context_);
}

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

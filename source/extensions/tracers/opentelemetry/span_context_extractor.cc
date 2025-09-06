#include "source/extensions/tracers/opentelemetry/span_context_extractor.h"

#include "envoy/tracing/tracer.h"

#include "source/common/http/header_map_impl.h"
#include "source/common/tracing/trace_context_impl.h"
#include "source/extensions/tracers/opentelemetry/span_context.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

using W3cConstants = Envoy::Extensions::Propagators::W3c::W3cConstants;
using TraceContextPropagator =
    Envoy::Extensions::Propagators::W3c::TraceContext::TraceContextPropagator;

SpanContextExtractor::SpanContextExtractor(Tracing::TraceContext& trace_context)
    : trace_context_(trace_context) {}

SpanContextExtractor::~SpanContextExtractor() = default;

bool SpanContextExtractor::propagationHeaderPresent() {
  return propagator_.hasTraceParent(trace_context_);
}

absl::StatusOr<SpanContext> SpanContextExtractor::extractSpanContext() {
  // Extract traceparent using the W3C propagator
  auto traceparent = propagator_.extractTraceParent(trace_context_);
  if (!traceparent.has_value()) {
    return absl::InvalidArgumentError("No traceparent header found");
  }

  // Parse using the W3C propagator
  auto parsed = propagator_.parseTraceParent(traceparent.value());
  if (!parsed.ok()) {
    return parsed.status();
  }

  // Extract tracestate if present
  auto tracestate = propagator_.extractTraceState(trace_context_);
  std::string tracestate_str = tracestate.value_or("");

  // Create SpanContext from parsed data
  SpanContext parent_context(parsed->version, parsed->trace_id, parsed->span_id, parsed->sampled,
                             tracestate_str);
  return parent_context;
}
}

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

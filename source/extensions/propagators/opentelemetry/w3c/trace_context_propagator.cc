#include "source/extensions/propagators/opentelemetry/w3c/trace_context_propagator.h"

#include "source/extensions/propagators/trace_context.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace OpenTelemetry {

namespace {
constexpr absl::string_view kDefaultVersion = "00";
}

absl::StatusOr<SpanContext> W3CTraceContextPropagator::extract(const Tracing::TraceContext& trace_context) {
  // Use the base W3C propagator to extract generic span context (Gang of Four Adapter pattern)
  auto generic_result = base_propagator_.extract(trace_context);
  if (!generic_result.ok()) {
    return generic_result.status();
  }

  // Convert generic span context to OpenTelemetry span context
  return convertFromGeneric(generic_result.value());
}

void W3CTraceContextPropagator::inject(const SpanContext& span_context, Tracing::TraceContext& trace_context) {
  // Convert OpenTelemetry span context to generic span context
  auto generic_span_context = convertToGeneric(span_context);
  
  // Use the base W3C propagator to inject (Gang of Four Adapter pattern)
  base_propagator_.inject(generic_span_context, trace_context);
}

std::vector<std::string> W3CTraceContextPropagator::fields() const {
  return base_propagator_.fields();
}

std::string W3CTraceContextPropagator::name() const {
  return base_propagator_.name();
}

SpanContext W3CTraceContextPropagator::convertFromGeneric(const Extensions::Propagators::SpanContext& generic_span_context) {
  // Convert from generic SpanContext to OpenTelemetry SpanContext
  return SpanContext(
    kDefaultVersion,
    generic_span_context.traceId().toHex(),
    generic_span_context.spanId().toHex(),
    generic_span_context.sampled(),
    generic_span_context.tracestate()
  );
}

Extensions::Propagators::SpanContext W3CTraceContextPropagator::convertToGeneric(const SpanContext& otel_span_context) {
  // Convert from OpenTelemetry SpanContext to generic SpanContext
  Extensions::Propagators::TraceId trace_id(otel_span_context.traceId());
  Extensions::Propagators::SpanId span_id(otel_span_context.spanId());
  Extensions::Propagators::TraceFlags trace_flags;
  trace_flags.setSampled(otel_span_context.sampled());
  
  return Extensions::Propagators::SpanContext(
    trace_id, 
    span_id, 
    trace_flags, 
    absl::nullopt, // parent span ID not used in OpenTelemetry SpanContext
    otel_span_context.tracestate()
  );
}

} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

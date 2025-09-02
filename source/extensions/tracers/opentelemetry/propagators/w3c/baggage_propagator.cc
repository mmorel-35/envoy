#include "source/extensions/tracers/opentelemetry/propagators/w3c/baggage_propagator.h"

#include "source/common/common/macros.h"
#include "source/common/tracing/trace_context_impl.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

BaggagePropagator::BaggagePropagator() : baggage_header_("baggage") {}

absl::StatusOr<SpanContext> BaggagePropagator::extract(const Tracing::TraceContext& trace_context) {
  UNREFERENCED_PARAMETER(trace_context);
  // Baggage propagator doesn't extract trace context per OpenTelemetry specification
  // It only handles baggage propagation, which is orthogonal to trace context
  // This should return an error to indicate no trace context was extracted
  // but without affecting any previously valid trace context per the spec requirement:
  // "If a value can not be parsed from the carrier, the implementation MUST NOT store a new value
  // in the Context"
  return absl::InvalidArgumentError("Baggage propagator cannot extract span context");
}

void BaggagePropagator::inject(const SpanContext& span_context,
                               Tracing::TraceContext& trace_context) {
  UNREFERENCED_PARAMETER(span_context);
  UNREFERENCED_PARAMETER(trace_context);
  // Per OpenTelemetry specification, baggage propagator should inject baggage headers
  // if baggage is present. Since Envoy's current SpanContext doesn't store baggage,
  // this is currently a no-op but the structure is ready for future baggage support.
  //
  // In a full implementation following W3C Baggage spec, this would:
  // 1. Extract baggage from the current context
  // 2. Serialize baggage entries to "baggage" header format
  // 3. Inject the "baggage" header into the trace context
  //
  // For now, this ensures the propagator can be safely included in multi-propagator
  // configurations without breaking injection behavior.
}

std::vector<std::string> BaggagePropagator::fields() const { return {"baggage"}; }

std::string BaggagePropagator::name() const { return "baggage"; }

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

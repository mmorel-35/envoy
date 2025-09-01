#include "source/extensions/tracers/opentelemetry/baggage_propagator.h"

#include "source/common/tracing/trace_context_impl.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

BaggagePropagator::BaggagePropagator() : baggage_header_("baggage") {}

absl::StatusOr<SpanContext> BaggagePropagator::extract(const Tracing::TraceContext& trace_context) {
  // Baggage propagator doesn't extract trace context, only baggage
  // This is expected to be used with other propagators that extract trace context
  auto baggage_header = baggage_header_.get(trace_context);
  if (!baggage_header.has_value()) {
    return absl::InvalidArgumentError("No baggage header found");
  }

  // For now, baggage extraction doesn't affect span context creation
  // In a full implementation, baggage would be stored separately and associated with spans
  return absl::InvalidArgumentError("Baggage propagator doesn't extract trace context");
}

void BaggagePropagator::inject(const SpanContext& span_context, Tracing::TraceContext& trace_context) {
  // Baggage injection would require baggage to be stored in the span context
  // For now, this is a no-op since we don't have baggage support in SpanContext
  // In a full implementation, baggage would be injected here
}

std::vector<std::string> BaggagePropagator::fields() const {
  return {"baggage"};
}

std::string BaggagePropagator::name() const {
  return "baggage";
}

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
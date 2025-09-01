#include "source/extensions/tracers/opentelemetry/propagators/propagator.h"

#include "source/common/common/logger.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

CompositePropagator::CompositePropagator(std::vector<TextMapPropagatorPtr> propagators)
    : propagators_(std::move(propagators)) {}

absl::StatusOr<SpanContext>
CompositePropagator::extract(const Tracing::TraceContext& trace_context) {
  for (const auto& propagator : propagators_) {
    auto result = propagator->extract(trace_context);
    if (result.ok()) {
      ENVOY_LOG(debug, "Successfully extracted span context using {} propagator",
                propagator->name());
      return result;
    }
    ENVOY_LOG(trace, "Failed to extract span context using {} propagator: {}", propagator->name(),
              result.status().message());
  }

  return absl::InvalidArgumentError("No propagator could extract span context");
}

void CompositePropagator::inject(const SpanContext& span_context,
                                 Tracing::TraceContext& trace_context) {
  for (const auto& propagator : propagators_) {
    // Skip baggage propagator for injection since it doesn't inject trace context
    if (propagator->name() == "baggage") {
      continue;
    }
    propagator->inject(span_context, trace_context);
    ENVOY_LOG(trace, "Injected span context using {} propagator", propagator->name());
  }
}

bool CompositePropagator::propagationHeaderPresent(const Tracing::TraceContext& trace_context) {
  for (const auto& propagator : propagators_) {
    // Skip baggage propagator for header presence check since it doesn't carry trace context
    if (propagator->name() == "baggage") {
      continue;
    }

    auto result = propagator->extract(trace_context);
    if (result.ok()) {
      return true;
    }
  }
  return false;
}

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

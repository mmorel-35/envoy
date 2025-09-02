#pragma once

#include "source/extensions/tracers/opentelemetry/propagators/propagator.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

/**
 * W3C Baggage propagator.
 * Handles baggage header for cross-cutting concerns.
 * Note: This propagator only handles baggage, not trace context.
 * It should be used in combination with trace context propagators.
 * See: https://w3c.github.io/baggage/
 */
class BaggagePropagator : public TextMapPropagator {
public:
  BaggagePropagator();

  // TextMapPropagator
  absl::StatusOr<SpanContext> extract(const Tracing::TraceContext& trace_context) override;
  void inject(const SpanContext& span_context, Tracing::TraceContext& trace_context) override;
  std::vector<std::string> fields() const override;
  std::string name() const override;

private:
  const Tracing::TraceContextHandler baggage_header_;
};

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

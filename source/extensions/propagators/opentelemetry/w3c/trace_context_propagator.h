#pragma once

#include "source/extensions/propagators/opentelemetry/propagator.h"
#include "source/extensions/propagators/w3c/trace_context_propagator.h"
#include "source/common/tracing/trace_context_impl.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace OpenTelemetry {

// Use the OpenTelemetry SpanContext type for this interface
using SpanContext = Extensions::Tracers::OpenTelemetry::SpanContext;

/**
 * OpenTelemetry W3C Trace Context propagator that reuses the base W3C propagator implementation.
 * Applies Gang of Four Adapter pattern to eliminate code duplication.
 * Handles traceparent and tracestate headers through composition.
 * Converts between generic SpanContext and OpenTelemetry SpanContext types.
 * See: https://www.w3.org/TR/trace-context/
 */
class W3CTraceContextPropagator : public TextMapPropagator {
public:
  W3CTraceContextPropagator() = default;

  // TextMapPropagator
  absl::StatusOr<SpanContext> extract(const Tracing::TraceContext& trace_context) override;
  void inject(const SpanContext& span_context, Tracing::TraceContext& trace_context) override;
  std::vector<std::string> fields() const override;
  std::string name() const override;

private:
  // Conversion helpers (Gang of Four Adapter pattern)
  static SpanContext convertFromGeneric(const Extensions::Propagators::SpanContext& generic_span_context);
  static Extensions::Propagators::SpanContext convertToGeneric(const SpanContext& otel_span_context);

  // Base W3C trace context propagator that handles the actual W3C logic (Gang of Four Composition pattern)
  Extensions::Propagators::W3C::TraceContextPropagator base_propagator_;
};

} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

#pragma once

#include "source/extensions/propagators/zipkin/propagator.h"
#include "source/extensions/propagators/w3c/trace_context_propagator.h"
#include "source/extensions/tracers/common/utils/trace.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace Zipkin {

/**
 * Zipkin W3C Trace Context propagator that reuses the base W3C propagator implementation.
 * Eliminates code duplication through composition and type conversion.
 * Implements the W3C Trace Context specification while using Zipkin-specific types through composition.
 *
 * Handles W3C traceparent and tracestate headers:
 * - traceparent: version-trace_id-parent_id-trace_flags
 * - tracestate: vendor-specific state information
 *
 * Converts between W3C format and Zipkin SpanContext types.
 */
class W3CTraceContextPropagator : public TextMapPropagator {
public:
  W3CTraceContextPropagator() = default;

  // TextMapPropagator interface
  absl::StatusOr<Extensions::Tracers::Zipkin::SpanContext>
  extract(const Tracing::TraceContext& trace_context) override;

  void inject(const Extensions::Tracers::Zipkin::SpanContext& span_context,
              Tracing::TraceContext& trace_context) override;

  std::vector<std::string> fields() const override;

  std::string name() const override { return "tracecontext"; }

private:
  // Conversion helpers for type adaptation
  static Extensions::Tracers::Zipkin::SpanContext convertFromGeneric(const Extensions::Propagators::SpanContext& generic_span_context);
  static Extensions::Propagators::SpanContext convertToGeneric(const Extensions::Tracers::Zipkin::SpanContext& zipkin_span_context);

  // Base W3C trace context propagator that handles the actual W3C protocol logic
  Extensions::Propagators::W3C::TraceContextPropagator base_propagator_;
};

} // namespace Zipkin
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

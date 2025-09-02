#pragma once

#include "source/extensions/propagators/zipkin/propagator.h"
#include "source/extensions/propagators/b3/propagator.h"
#include "source/extensions/tracers/common/utils/trace.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace Zipkin {

/**
 * Zipkin B3 propagator that reuses the base B3 propagator implementation.
 * Implements the B3 propagation specification while using Zipkin-specific types through composition.
 *
 * Supports both single and multi-header B3 formats:
 * - Single: b3: {trace_id}-{span_id}-{sampling_state}-{parent_span_id}
 * - Multi: X-B3-TraceId, X-B3-SpanId, X-B3-ParentSpanId, X-B3-Sampled, X-B3-Flags
 *
 * Handles special B3 features:
 * - Debug sampling ("d" flag)
 * - Sampling-only headers ("0", "1", "d" without full context)
 * - Parent span ID extraction for Zipkin compatibility
 */
class B3Propagator : public TextMapPropagator {
public:
  B3Propagator() = default;

  // TextMapPropagator interface
  absl::StatusOr<Extensions::Tracers::Zipkin::SpanContext>
  extract(const Tracing::TraceContext& trace_context) override;

  void inject(const Extensions::Tracers::Zipkin::SpanContext& span_context,
              Tracing::TraceContext& trace_context) override;

  std::vector<std::string> fields() const override;

  std::string name() const override { return "b3"; }

private:
  // Conversion helpers
  static Extensions::Tracers::Zipkin::SpanContext convertFromGeneric(const Extensions::Propagators::SpanContext& generic_span_context);
  static Extensions::Propagators::SpanContext convertToGeneric(const Extensions::Tracers::Zipkin::SpanContext& zipkin_span_context);

  // Base B3 propagator that handles the actual B3 logic
  Extensions::Propagators::B3::B3Propagator base_propagator_;
};

} // namespace Zipkin
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

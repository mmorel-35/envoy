#pragma once

#include "source/extensions/propagators/opentelemetry/propagator.h"
#include "source/extensions/propagators/b3/propagator.h"
#include "source/common/tracing/trace_context_impl.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace OpenTelemetry {

/**
 * OpenTelemetry B3 propagator that reuses the base B3 propagator implementation.
 * Supports both single header (b3) and multi-header (X-B3-*) formats through composition.
 * Converts between generic SpanContext and OpenTelemetry SpanContext types.
 * See: https://github.com/openzipkin/b3-propagation
 */
class B3Propagator : public TextMapPropagator {
public:
  B3Propagator();

  // TextMapPropagator
  absl::StatusOr<SpanContext> extract(const Tracing::TraceContext& trace_context) override;
  void inject(const SpanContext& span_context, Tracing::TraceContext& trace_context) override;
  std::vector<std::string> fields() const override;
  std::string name() const override;

private:
  // Conversion helpers
  static SpanContext convertFromGeneric(const Extensions::Propagators::SpanContext& generic_span_context);
  static Extensions::Propagators::SpanContext convertToGeneric(const SpanContext& otel_span_context);

  // Base B3 propagator that handles the actual B3 logic
  Extensions::Propagators::B3::B3Propagator base_propagator_;
};

} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

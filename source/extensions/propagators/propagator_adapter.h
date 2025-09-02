#pragma once

#include "source/extensions/propagators/propagator.h"
#include "source/extensions/propagators/generic_propagator.h" 
#include "source/extensions/propagators/type_converter.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {

/**
 * Adapter that allows generic propagators to work with the existing OpenTelemetry-based
 * propagator interface. This enables backward compatibility while migrating to generic types.
 */
class GenericToOtelPropagatorAdapter : public TextMapPropagator {
public:
  explicit GenericToOtelPropagatorAdapter(GenericPropagatorPtr generic_propagator);

  // TextMapPropagator interface
  absl::StatusOr<Tracers::OpenTelemetry::SpanContext> extract(const Tracing::TraceContext& trace_context) override;
  void inject(const Tracers::OpenTelemetry::SpanContext& span_context, Tracing::TraceContext& trace_context) override;
  std::vector<std::string> fields() const override;
  std::string name() const override;

private:
  GenericPropagatorPtr generic_propagator_;
};

/**
 * Adapter that allows OpenTelemetry-based propagators to work with the generic
 * propagator interface. This enables gradual migration and testing.
 */
class OtelToGenericPropagatorAdapter : public GenericPropagator {
public:
  explicit OtelToGenericPropagatorAdapter(TextMapPropagatorPtr otel_propagator);

  // GenericPropagator interface
  absl::StatusOr<SpanContext> extract(const Tracing::TraceContext& trace_context) override;
  void inject(const SpanContext& span_context, Tracing::TraceContext& trace_context) override;
  std::vector<std::string> fields() const override;
  std::string name() const override;

private:
  TextMapPropagatorPtr otel_propagator_;
};

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
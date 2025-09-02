#include "source/extensions/propagators/propagator_adapter.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {

// GenericToOtelPropagatorAdapter implementation
GenericToOtelPropagatorAdapter::GenericToOtelPropagatorAdapter(GenericPropagatorPtr generic_propagator)
    : generic_propagator_(std::move(generic_propagator)) {}

absl::StatusOr<Tracers::OpenTelemetry::SpanContext> 
GenericToOtelPropagatorAdapter::extract(const Tracing::TraceContext& trace_context) {
  auto generic_result = generic_propagator_->extract(trace_context);
  if (!generic_result.ok()) {
    return generic_result.status();
  }
  
  return TypeConverter::toOpenTelemetrySpanContext(generic_result.value());
}

void GenericToOtelPropagatorAdapter::inject(const Tracers::OpenTelemetry::SpanContext& span_context, 
                                           Tracing::TraceContext& trace_context) {
  SpanContext generic_context = TypeConverter::fromOpenTelemetrySpanContext(span_context);
  generic_propagator_->inject(generic_context, trace_context);
}

std::vector<std::string> GenericToOtelPropagatorAdapter::fields() const {
  return generic_propagator_->fields();
}

std::string GenericToOtelPropagatorAdapter::name() const {
  return generic_propagator_->name();
}

// OtelToGenericPropagatorAdapter implementation  
OtelToGenericPropagatorAdapter::OtelToGenericPropagatorAdapter(TextMapPropagatorPtr otel_propagator)
    : otel_propagator_(std::move(otel_propagator)) {}

absl::StatusOr<SpanContext> 
OtelToGenericPropagatorAdapter::extract(const Tracing::TraceContext& trace_context) {
  auto otel_result = otel_propagator_->extract(trace_context);
  if (!otel_result.ok()) {
    return otel_result.status();
  }
  
  return TypeConverter::fromOpenTelemetrySpanContext(otel_result.value());
}

void OtelToGenericPropagatorAdapter::inject(const SpanContext& span_context, 
                                           Tracing::TraceContext& trace_context) {
  Tracers::OpenTelemetry::SpanContext otel_context = TypeConverter::toOpenTelemetrySpanContext(span_context);
  otel_propagator_->inject(otel_context, trace_context);
}

std::vector<std::string> OtelToGenericPropagatorAdapter::fields() const {
  return otel_propagator_->fields();
}

std::string OtelToGenericPropagatorAdapter::name() const {
  return otel_propagator_->name();
}

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
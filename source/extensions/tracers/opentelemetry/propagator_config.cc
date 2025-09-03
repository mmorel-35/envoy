#include "source/extensions/tracers/opentelemetry/propagator_config.h"

#include "source/common/common/logger.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

PropagatorConfig::PropagatorConfig(const envoy::config::trace::v3::OpenTelemetryConfig& config, Api::Api& api) {
  propagator_config_ = Extensions::Propagators::OpenTelemetry::Propagator::createConfig(config, api);
}

bool PropagatorConfig::propagationHeaderPresent(const Tracing::TraceContext& trace_context) {
  return Extensions::Propagators::OpenTelemetry::TracingHelper::propagationHeaderPresent(trace_context, propagator_config_);
}

absl::StatusOr<SpanContext> PropagatorConfig::extractSpanContext(const Tracing::TraceContext& trace_context) {
  auto composite_result = Extensions::Propagators::OpenTelemetry::TracingHelper::extractWithConfig(trace_context, propagator_config_);
  if (!composite_result.ok()) {
    return composite_result.status();
  }
  
  return convertFromComposite(composite_result.value());
}

void PropagatorConfig::injectSpanContext(const SpanContext& span_context, Tracing::TraceContext& trace_context) {
  auto composite_context = convertToComposite(span_context);
  auto result = Extensions::Propagators::OpenTelemetry::TracingHelper::injectWithConfig(composite_context, trace_context, propagator_config_);
  if (!result.ok()) {
    ENVOY_LOG(warn, "Failed to inject span context: {}", result.message());
  }
}

absl::StatusOr<SpanContext> PropagatorConfig::convertFromComposite(
    const Extensions::Propagators::OpenTelemetry::CompositeTraceContext& composite_context) {
  
  // Create OpenTelemetry SpanContext from composite context
  SpanContext span_context(
    "00",                                    // version (default for compatibility)
    composite_context.getTraceId(),         // trace_id
    composite_context.getSpanId(),          // span_id
    composite_context.isSampled(),          // sampled
    composite_context.getTraceState()       // trace_state
  );
  
  return span_context;
}

Extensions::Propagators::OpenTelemetry::CompositeTraceContext PropagatorConfig::convertToComposite(
    const SpanContext& span_context) {
  
  // Use TracingHelper to create composite context from span context data
  auto result = Extensions::Propagators::OpenTelemetry::TracingHelper::createFromTracerData(
    span_context.traceId(),
    span_context.spanId(),
    "",  // no parent span id available from SpanContext
    span_context.sampled(),
    span_context.tracestate(),
    Extensions::Propagators::OpenTelemetry::TraceFormat::W3C
  );
  
  if (result.ok()) {
    return result.value();
  } else {
    // Return empty context if conversion fails
    return Extensions::Propagators::OpenTelemetry::CompositeTraceContext();
  }
}

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
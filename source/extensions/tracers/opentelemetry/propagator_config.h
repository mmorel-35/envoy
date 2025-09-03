#pragma once

#include "envoy/api/api.h"
#include "envoy/config/trace/v3/opentelemetry.pb.h"
#include "envoy/tracing/trace_context.h"

#include "source/common/common/statusor.h"
#include "source/extensions/propagators/opentelemetry/propagator.h"
#include "source/extensions/tracers/opentelemetry/span_context.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

/**
 * PropagatorConfig provides a bridge between the OpenTelemetry tracer and the 
 * configurable propagators in source/extensions/propagators/opentelemetry.
 * 
 * This class maintains the same interface as the original PropagatorConfig
 * for backward compatibility, while delegating to the new composite propagator.
 */
class PropagatorConfig {
public:
  /**
   * Constructor that reads configuration from proto and environment
   */
  PropagatorConfig(const envoy::config::trace::v3::OpenTelemetryConfig& config, Api::Api& api);

  /**
   * Extract span context using configured propagators
   * @param trace_context The trace context to extract from
   * @return SpanContext if extraction successful
   */
  absl::StatusOr<SpanContext> extractSpanContext(const Tracing::TraceContext& trace_context);

  /**
   * Check if any propagation headers are present
   * @param trace_context The trace context to check
   * @return true if propagation headers found
   */
  bool propagationHeaderPresent(const Tracing::TraceContext& trace_context);

  /**
   * Inject span context using configured propagators
   * @param span_context The span context to inject
   * @param trace_context The trace context to inject into
   */
  void injectSpanContext(const SpanContext& span_context, Tracing::TraceContext& trace_context);

private:
  /**
   * Convert CompositeTraceContext to OpenTelemetry SpanContext
   */
  absl::StatusOr<SpanContext> convertFromComposite(
      const Extensions::Propagators::OpenTelemetry::CompositeTraceContext& composite_context);

  /**
   * Convert OpenTelemetry SpanContext to CompositeTraceContext
   */
  Extensions::Propagators::OpenTelemetry::CompositeTraceContext convertToComposite(
      const SpanContext& span_context);

  Extensions::Propagators::OpenTelemetry::Propagator::Config propagator_config_;
};

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
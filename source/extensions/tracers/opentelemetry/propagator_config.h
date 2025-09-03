#pragma once

#include <string>
#include <vector>

#include "envoy/api/api.h"
#include "envoy/config/trace/v3/opentelemetry.pb.h"

#include "source/common/common/statusor.h"
#include "source/extensions/propagators/b3/propagator.h"
#include "source/extensions/propagators/opentelemetry/propagator.h"
#include "source/extensions/propagators/w3c/propagator.h"
#include "source/extensions/tracers/opentelemetry/span_context.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

/**
 * Supported propagator types for OpenTelemetry tracer configuration
 */
enum class PropagatorType {
  TraceContext,  // W3C Trace Context
  Baggage,       // W3C Baggage  
  B3,            // B3 single header
  B3Multi,       // B3 multiple headers
  None           // No propagation
};

/**
 * Configuration manager for OpenTelemetry propagators.
 * Handles configuration from proto and environment variables,
 * with fallback to default behavior for backward compatibility.
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
   * Parse propagator configuration from proto and environment
   */
  void parsePropagatorConfig(const envoy::config::trace::v3::OpenTelemetryConfig& config, Api::Api& api);

  /**
   * Convert string to propagator type
   */
  absl::StatusOr<PropagatorType> stringToPropagatorType(const std::string& propagator_str);

  /**
   * Convert SpanContext from W3C format
   */
  absl::StatusOr<SpanContext> convertFromW3C(
      const Extensions::Propagators::W3C::TraceContext& w3c_context);

  /**
   * Convert SpanContext from B3 format  
   */
  absl::StatusOr<SpanContext> convertFromB3(
      const Extensions::Propagators::B3::TraceContext& b3_context);

  /**
   * Convert SpanContext to W3C format for injection
   */
  Extensions::Propagators::W3C::TraceContext convertToW3C(const SpanContext& span_context);

  /**
   * Convert SpanContext to B3 format for injection
   */
  Extensions::Propagators::B3::TraceContext convertToB3(const SpanContext& span_context);

  std::vector<PropagatorType> propagators_;
  bool trace_context_enabled_;
  bool baggage_enabled_;
  bool b3_enabled_;
  bool b3_multi_enabled_;
};

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
#pragma once

#include "envoy/api/api.h"

#include "source/extensions/tracers/opentelemetry/propagators/propagator.h"
#include "source/extensions/tracers/opentelemetry/propagators/w3c/w3c_trace_context_propagator.h"
#include "source/extensions/tracers/opentelemetry/propagators/b3/b3_propagator.h"
#include "source/extensions/tracers/opentelemetry/propagators/w3c/baggage_propagator.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

/**
 * Factory for creating propagators from configuration.
 */
class PropagatorFactory {
public:
  /**
   * Create a composite propagator from configuration and environment variables.
   * Priority: explicit config > OTEL_PROPAGATORS env var > default (tracecontext).
   * @param propagator_names List of propagator names from config (e.g., "tracecontext", "b3",
   * "baggage").
   * @param api API interface for reading environment variables.
   * @return CompositePropagator containing the specified propagators.
   */
  static CompositePropagatorPtr createPropagators(const std::vector<std::string>& propagator_names,
                                                  Api::Api& api);

  /**
   * Create a composite propagator from a list of propagator names.
   * @param propagator_names List of propagator names (e.g., "tracecontext", "b3", "baggage").
   * @return CompositePropagator containing the specified propagators.
   */
  static CompositePropagatorPtr createPropagators(const std::vector<std::string>& propagator_names);

  /**
   * Get the default propagator configuration (W3C Trace Context only).
   * @return CompositePropagator with default configuration.
   */
  static CompositePropagatorPtr createDefaultPropagators();

  /**
   * Parse OTEL_PROPAGATORS environment variable format.
   * @param env_value The environment variable value (comma-separated propagator names).
   * @return Vector of propagator names parsed from the environment variable.
   */
  static std::vector<std::string> parseOtelPropagatorsEnv(const std::string& env_value);

private:
  static TextMapPropagatorPtr createPropagator(const std::string& name);
};

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

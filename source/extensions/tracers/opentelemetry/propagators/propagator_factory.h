#pragma once

#include "source/extensions/tracers/opentelemetry/propagators/propagator.h"
#include "source/extensions/tracers/opentelemetry/propagators/w3c_trace_context_propagator.h"
#include "source/extensions/tracers/opentelemetry/propagators/b3_propagator.h"
#include "source/extensions/tracers/opentelemetry/propagators/baggage_propagator.h"

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

private:
  static TextMapPropagatorPtr createPropagator(const std::string& name);
};

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

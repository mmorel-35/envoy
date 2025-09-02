#pragma once

#include "source/extensions/propagators/generic_propagator.h"
#include "source/extensions/propagators/b3/b3_propagator.h"
#include "source/extensions/propagators/w3c/trace_context_propagator.h"
#include "source/extensions/propagators/w3c/baggage_propagator.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {

/**
 * Factory for creating generic propagators that are tracer-agnostic.
 * These propagators implement their specifications exactly without dependencies
 * on any specific tracer implementation.
 */
class GenericPropagatorFactory {
public:
  /**
   * Create a generic propagator by name.
   * @param name The propagator name ("b3", "tracecontext", "baggage").
   * @return Generic propagator or nullptr if name is unknown.
   */
  static GenericPropagatorPtr createGenericPropagator(const std::string& name);

  /**
   * Create a composite propagator from a list of propagator names.
   * @param propagator_names List of propagator names to combine.
   * @return Composite generic propagator.
   */
  static GenericCompositePropagatorPtr createCompositeGenericPropagator(
      const std::vector<std::string>& propagator_names);

  /**
   * Parse OpenTelemetry OTEL_PROPAGATORS environment variable format.
   * @param otel_propagators_env The environment variable value.
   * @return List of propagator names.
   */
  static std::vector<std::string> parseOtelPropagatorsEnv(const std::string& otel_propagators_env);

  /**
   * Get the list of supported generic propagator names.
   * @return List of supported propagator names.
   */
  static std::vector<std::string> getSupportedPropagatorNames();
};

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
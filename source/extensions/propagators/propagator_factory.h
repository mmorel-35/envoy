#pragma once

#include <mutex>

#include "envoy/api/api.h"

#include "source/common/common/logger.h"
#include "source/extensions/propagators/propagator.h"
#include "source/extensions/propagators/generic_propagator.h"
#include "source/extensions/propagators/generic_propagator_factory.h"
#include "source/extensions/propagators/propagator_adapter.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {

/**
 * Factory for creating propagators from configuration.
 * Supports both legacy OpenTelemetry-coupled and new generic propagator interfaces.
 */
class PropagatorFactory : public Logger::Loggable<Logger::Id::tracing> {
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
   * Create a generic composite propagator from a list of propagator names.
   * This creates propagators using the new generic interface that is tracer-agnostic.
   * @param propagator_names List of propagator names (e.g., "tracecontext", "b3", "baggage").
   * @return GenericCompositePropagator containing the specified propagators.
   */
  static GenericCompositePropagatorPtr createGenericPropagators(const std::vector<std::string>& propagator_names);

  /**
   * Create a generic composite propagator from configuration and environment variables.
   * @param propagator_names List of propagator names from config.
   * @param api API interface for reading environment variables.
   * @return GenericCompositePropagator containing the specified propagators.
   */
  static GenericCompositePropagatorPtr createGenericPropagators(const std::vector<std::string>& propagator_names,
                                                                Api::Api& api);

  /**
   * Get the default propagator configuration (W3C Trace Context only).
   * @return CompositePropagator with default configuration.
   */
  static CompositePropagatorPtr createDefaultPropagators();

  /**
   * Get the default generic propagator configuration (W3C Trace Context only).
   * @return GenericCompositePropagator with default configuration.
   */
  static GenericCompositePropagatorPtr createDefaultGenericPropagators();

  /**
   * Parse OTEL_PROPAGATORS environment variable format.
   * @param env_value The environment variable value (comma-separated propagator names).
   * @return Vector of propagator names parsed from the environment variable.
   */
  static std::vector<std::string> parseOtelPropagatorsEnv(const std::string& env_value);

  /**
   * Get the global text map propagator. Provides OpenTelemetry specification compliance
   * for global propagator access. Returns the default propagator if none is set.
   * @return The current global TextMapPropagator instance.
   */
  static CompositePropagator& getGlobalTextMapPropagator();

  /**
   * Set the global text map propagator. Provides OpenTelemetry specification compliance
   * for global propagator configuration.
   * @param propagator The propagator to set as global. Must not be null.
   */
  static void setGlobalTextMapPropagator(CompositePropagatorPtr propagator);

private:
  static TextMapPropagatorPtr createPropagator(const std::string& name);

  // Global propagator instance for OpenTelemetry specification compliance
  static CompositePropagatorPtr global_propagator_;
  static std::once_flag global_propagator_once_;
};

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
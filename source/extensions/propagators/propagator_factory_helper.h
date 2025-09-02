#pragma once

#include "source/common/common/logger.h"

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

namespace Envoy {
namespace Extensions {
namespace Propagators {

/**
 * Gang of Four Strategy pattern helper for propagator factory implementations.
 * This eliminates code duplication between OpenTelemetry and Zipkin factories
 * by providing a common algorithm with pluggable creation strategies.
 */
template<typename TextMapPropagatorPtr, typename CompositePropagatorPtr>
class PropagatorFactoryHelper : public Logger::Loggable<Logger::Id::tracing> {
public:
  using PropagatorCreatorMap = std::unordered_map<std::string, std::function<TextMapPropagatorPtr()>>;

  /**
   * Create a composite propagator from a list of propagator names using Strategy pattern.
   * @param propagator_names List of propagator names to create
   * @param creators Map of propagator name to creator function (Strategy pattern)
   * @param default_propagators Function to create default propagators
   */
  static CompositePropagatorPtr createPropagators(
      const std::vector<std::string>& propagator_names,
      const PropagatorCreatorMap& creators,
      std::function<CompositePropagatorPtr()> default_propagators) {
    
    std::vector<TextMapPropagatorPtr> propagators;

    for (const auto& name : propagator_names) {
      auto it = creators.find(name);
      if (it != creators.end()) {
        auto propagator = it->second();
        if (propagator) {
          propagators.push_back(std::move(propagator));
        }
      } else {
        ENVOY_LOG(warn, "Unknown propagator name: {}. Ignoring.", name);
      }
    }

    if (propagators.empty()) {
      ENVOY_LOG(info, "No valid propagators specified, using default");
      return default_propagators();
    }

    // Use template argument deduction to create the composite propagator
    using CompositePropagatorType = typename CompositePropagatorPtr::element_type;
    return std::make_unique<CompositePropagatorType>(std::move(propagators));
  }
};

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
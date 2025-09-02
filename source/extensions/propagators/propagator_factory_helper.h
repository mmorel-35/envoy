#pragma once

#include "source/common/common/logger.h"

#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace Envoy {
namespace Extensions {
namespace Propagators {

/**
 * Shared utility for creating composite propagators with common error handling.
 * Eliminates code duplication between propagator factories.
 */
template<typename TextMapPropagatorPtr, typename CompositePropagatorPtr>
CompositePropagatorPtr createCompositePropagator(
    const std::vector<std::string>& propagator_names,
    std::function<TextMapPropagatorPtr(const std::string&)> create_propagator,
    std::function<CompositePropagatorPtr()> create_default) {
  
  std::vector<TextMapPropagatorPtr> propagators;

  for (const auto& name : propagator_names) {
    auto propagator = create_propagator(name);
    if (propagator) {
      propagators.push_back(std::move(propagator));
    } else {
      ENVOY_LOG(warn, "Unknown propagator name: {}. Ignoring.", name);
    }
  }

  if (propagators.empty()) {
    ENVOY_LOG(info, "No valid propagators specified, using default");
    return create_default();
  }

  using CompositePropagatorType = typename CompositePropagatorPtr::element_type;
  return std::make_unique<CompositePropagatorType>(std::move(propagators));
}

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
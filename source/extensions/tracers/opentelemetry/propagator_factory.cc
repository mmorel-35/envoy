#include "source/extensions/tracers/opentelemetry/propagator_factory.h"

#include "source/common/common/logger.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

CompositePropagatorPtr PropagatorFactory::createPropagators(const std::vector<std::string>& propagator_names) {
  std::vector<TextMapPropagatorPtr> propagators;
  
  for (const auto& name : propagator_names) {
    auto propagator = createPropagator(name);
    if (propagator) {
      propagators.push_back(std::move(propagator));
    } else {
      ENVOY_LOG(warn, "Unknown propagator name: {}. Ignoring.", name);
    }
  }
  
  if (propagators.empty()) {
    ENVOY_LOG(info, "No valid propagators specified, using default W3C Trace Context");
    return createDefaultPropagators();
  }
  
  return std::make_unique<CompositePropagator>(std::move(propagators));
}

CompositePropagatorPtr PropagatorFactory::createDefaultPropagators() {
  std::vector<TextMapPropagatorPtr> propagators;
  propagators.push_back(std::make_unique<W3CTraceContextPropagator>());
  return std::make_unique<CompositePropagator>(std::move(propagators));
}

TextMapPropagatorPtr PropagatorFactory::createPropagator(const std::string& name) {
  if (name == "tracecontext") {
    return std::make_unique<W3CTraceContextPropagator>();
  } else if (name == "b3") {
    return std::make_unique<B3Propagator>();
  } else if (name == "baggage") {
    return std::make_unique<BaggagePropagator>();
  }
  
  return nullptr;
}

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
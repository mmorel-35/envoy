#include "source/extensions/propagators/zipkin/propagator_factory.h"

#include "source/common/common/logger.h"
#include "source/extensions/propagators/opentelemetry/w3c/w3c_trace_context_propagator.h"
#include "source/extensions/propagators/opentelemetry/b3/b3_propagator.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace Zipkin {

Extensions::Propagators::OpenTelemetry::CompositePropagatorPtr
PropagatorFactory::createPropagators(const std::vector<std::string>& propagator_names) {
  std::vector<Extensions::Propagators::OpenTelemetry::TextMapPropagatorPtr> propagators;

  for (const auto& name : propagator_names) {
    auto propagator = createPropagator(name);
    if (propagator) {
      propagators.push_back(std::move(propagator));
    } else {
      ENVOY_LOG(warn, "Unknown propagator name: {}. Ignoring.", name);
    }
  }

  if (propagators.empty()) {
    ENVOY_LOG(info, "No valid propagators specified, using default B3 format for Zipkin");
    return createDefaultPropagators();
  }

  return std::make_unique<Extensions::Propagators::OpenTelemetry::CompositePropagator>(
      std::move(propagators));
}

Extensions::Propagators::OpenTelemetry::CompositePropagatorPtr
PropagatorFactory::createDefaultPropagators() {
  std::vector<Extensions::Propagators::OpenTelemetry::TextMapPropagatorPtr> propagators;
  // Zipkin defaults to B3 format
  propagators.push_back(std::make_unique<Extensions::Propagators::OpenTelemetry::B3Propagator>());
  return std::make_unique<Extensions::Propagators::OpenTelemetry::CompositePropagator>(
      std::move(propagators));
}

Extensions::Propagators::OpenTelemetry::TextMapPropagatorPtr
PropagatorFactory::createPropagator(const std::string& name) {
  if (name == "b3") {
    return std::make_unique<Extensions::Propagators::OpenTelemetry::B3Propagator>();
  } else if (name == "tracecontext") {
    return std::make_unique<Extensions::Propagators::OpenTelemetry::W3CTraceContextPropagator>();
  }

  return nullptr;
}

} // namespace Zipkin
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
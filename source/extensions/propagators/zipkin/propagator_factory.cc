#include "source/extensions/propagators/zipkin/propagator_factory.h"

#include "source/common/common/logger.h"
#include "source/extensions/propagators/zipkin/b3/propagator.h"
#include "source/extensions/propagators/zipkin/w3c/trace_context_propagator.h"
#include "source/extensions/propagators/propagator_factory_helper.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace Zipkin {

// Gang of Four Strategy pattern - define propagator creators as strategies
static PropagatorFactoryHelper<TextMapPropagatorPtr, CompositePropagatorPtr>::PropagatorCreatorMap
getZipkinPropagatorCreators() {
  return {
    {"b3", []() { return std::make_unique<B3Propagator>(); }},
    {"tracecontext", []() { return std::make_unique<W3CTraceContextPropagator>(); }}
  };
}

CompositePropagatorPtr
PropagatorFactory::createPropagators(const std::vector<std::string>& propagator_names) {
  // Gang of Four Strategy pattern - use helper with Zipkin-specific strategies
  return PropagatorFactoryHelper<TextMapPropagatorPtr, CompositePropagatorPtr>::createPropagators(
    propagator_names,
    getZipkinPropagatorCreators(),
    []() { return createDefaultPropagators(); }
  );
}

CompositePropagatorPtr PropagatorFactory::createDefaultPropagators() {
  std::vector<TextMapPropagatorPtr> propagators;
  // Zipkin defaults to B3 format
  propagators.push_back(std::make_unique<B3Propagator>());
  return std::make_unique<CompositePropagator>(std::move(propagators));
}

TextMapPropagatorPtr PropagatorFactory::createPropagator(const std::string& name) {
  // Use the strategy map to create propagators
  auto creators = getZipkinPropagatorCreators();
  auto it = creators.find(name);
  return it != creators.end() ? it->second() : nullptr;
}

} // namespace Zipkin
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

} // namespace Zipkin
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

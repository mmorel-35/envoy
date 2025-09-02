#include "source/extensions/propagators/zipkin/propagator_factory.h"

#include "source/common/common/logger.h"
#include "source/extensions/propagators/zipkin/b3/propagator.h"
#include "source/extensions/propagators/zipkin/w3c/trace_context_propagator.h"
#include "source/extensions/propagators/propagator_factory_helper.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace Zipkin {

CompositePropagatorPtr
PropagatorFactory::createPropagators(const std::vector<std::string>& propagator_names) {
  return createCompositePropagator<TextMapPropagatorPtr, CompositePropagatorPtr>(
    propagator_names,
    [](const std::string& name) { 
      if (name == "b3") {
        return std::make_unique<B3Propagator>();
      } else if (name == "tracecontext") {
        return std::make_unique<W3CTraceContextPropagator>();
      }
      return TextMapPropagatorPtr{};
    },
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
  if (name == "b3") {
    return std::make_unique<B3Propagator>();
  } else if (name == "tracecontext") {
    return std::make_unique<W3CTraceContextPropagator>();
  }
  return nullptr;
}

} // namespace Zipkin
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

} // namespace Zipkin
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

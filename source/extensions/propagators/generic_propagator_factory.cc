#include "source/extensions/propagators/generic_propagator_factory.h"

#include "absl/strings/str_split.h"
#include "absl/strings/ascii.h"
#include "source/common/common/logger.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {

GenericPropagatorPtr GenericPropagatorFactory::createGenericPropagator(const std::string& name) {
  std::string normalized_name = absl::AsciiStrToLower(name);
  
  if (normalized_name == "b3") {
    return std::make_unique<GenericB3Propagator>();
  } else if (normalized_name == "tracecontext") {
    return std::make_unique<GenericW3CTraceContextPropagator>();
  } else if (normalized_name == "baggage") {
    return std::make_unique<GenericW3CBaggagePropagator>();
  }
  
  return nullptr;
}

GenericCompositePropagatorPtr GenericPropagatorFactory::createCompositeGenericPropagator(
    const std::vector<std::string>& propagator_names) {
  std::vector<GenericPropagatorPtr> propagators;
  
  for (const auto& name : propagator_names) {
    auto propagator = createGenericPropagator(name);
    if (propagator) {
      propagators.push_back(std::move(propagator));
    }
  }
  
  return std::make_unique<GenericCompositePropagator>(std::move(propagators));
}

std::vector<std::string> GenericPropagatorFactory::parseOtelPropagatorsEnv(const std::string& otel_propagators_env) {
  if (otel_propagators_env.empty()) {
    // Default OpenTelemetry propagators according to specification
    return {"tracecontext", "baggage"};
  }
  
  std::vector<std::string> propagator_names;
  std::vector<std::string> parts = absl::StrSplit(otel_propagators_env, ',');
  
  for (const auto& part : parts) {
    std::string trimmed = absl::StripAsciiWhitespace(part);
    if (!trimmed.empty()) {
      propagator_names.push_back(trimmed);
    }
  }
  
  return propagator_names;
}

std::vector<std::string> GenericPropagatorFactory::getSupportedPropagatorNames() {
  return {"b3", "tracecontext", "baggage"};
}

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
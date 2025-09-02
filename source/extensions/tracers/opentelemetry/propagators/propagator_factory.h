#pragma once

#include "source/extensions/propagators/opentelemetry/propagator_factory.h"

// Re-export PropagatorFactory in the expected namespace for backward compatibility
namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

using PropagatorFactory = Propagators::OpenTelemetry::PropagatorFactory;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
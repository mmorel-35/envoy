#pragma once

#include "source/extensions/propagators/opentelemetry/b3/propagator.h"

// Re-export B3Propagator in the expected namespace for backward compatibility
namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

using B3Propagator = Propagators::OpenTelemetry::B3Propagator;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
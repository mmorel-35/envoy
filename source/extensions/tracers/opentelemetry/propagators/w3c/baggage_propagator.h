#pragma once

#include "source/extensions/propagators/opentelemetry/w3c/baggage_propagator.h"

// Re-export BaggagePropagator in the expected namespace for backward compatibility
namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

using BaggagePropagator = Propagators::OpenTelemetry::BaggagePropagator;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
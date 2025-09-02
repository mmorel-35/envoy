#pragma once

#include "source/extensions/propagators/opentelemetry/propagator.h"

// Re-export propagator classes in the expected namespace for backward compatibility
namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

using TextMapPropagator = Propagators::OpenTelemetry::TextMapPropagator;
using TextMapPropagatorPtr = Propagators::OpenTelemetry::TextMapPropagatorPtr;
using CompositePropagator = Propagators::OpenTelemetry::CompositePropagator;
using CompositePropagatorPtr = Propagators::OpenTelemetry::CompositePropagatorPtr;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
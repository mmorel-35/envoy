#pragma once

#include "source/extensions/propagators/opentelemetry/w3c/trace_context_propagator.h"

// Re-export W3CTraceContextPropagator in the expected namespace for backward compatibility
namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

using W3CTraceContextPropagator = Propagators::OpenTelemetry::W3CTraceContextPropagator;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
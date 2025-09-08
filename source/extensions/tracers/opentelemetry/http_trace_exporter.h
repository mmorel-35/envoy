#pragma once

// Compatibility header that forwards to the new OpenTelemetry SDK structure

#include "source/extensions/opentelemetry/exporters/otlp/http_trace_exporter.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

// Forward the new implementation to the old namespace for compatibility
using OpenTelemetryHttpTraceExporter = ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OpenTelemetryHttpTraceExporter;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

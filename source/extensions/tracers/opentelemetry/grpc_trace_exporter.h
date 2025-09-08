#pragma once

// Compatibility header that forwards to the new OpenTelemetry SDK structure

#include "source/extensions/opentelemetry/exporters/otlp/grpc_trace_exporter.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

// Forward the new implementation to the old namespace for compatibility
using OpenTelemetryGrpcTraceExporter = ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OpenTelemetryGrpcTraceExporter;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

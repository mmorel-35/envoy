#pragma once

// Compatibility header that forwards to the new OpenTelemetry SDK structure
// This allows existing tracer code to continue working with minimal changes

#include "source/extensions/opentelemetry/exporters/otlp/trace_exporter.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

// Forward the new implementations to the old namespace for compatibility
using OpenTelemetryTraceExporter = ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OpenTelemetryTraceExporter;
using OpenTelemetryTraceExporterPtr = ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OpenTelemetryTraceExporterPtr;
using ExportTraceServiceRequest = ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::ExportTraceServiceRequest;
using ExportTraceServiceResponse = ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::ExportTraceServiceResponse;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

#pragma once

// This file has moved to source/extensions/common/opentelemetry/exporters/otlp/trace_exporter.h
// It is kept here as a forwarding header for backward compatibility.

#include "source/extensions/common/opentelemetry/exporters/otlp/trace_exporter.h"

// Re-export the file-scope proto type aliases for backward compatibility with
// code that previously included this header and used unqualified names.
using opentelemetry::proto::collector::trace::v1::ExportTraceServiceRequest;
using opentelemetry::proto::collector::trace::v1::ExportTraceServiceResponse;

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

// Re-export types into the Tracers::OpenTelemetry namespace for backward compatibility.
using OpenTelemetryTraceExporter =
    ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OtlpTraceExporter;
using OpenTelemetryTraceExporterPtr =
    ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OtlpTraceExporterPtr;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

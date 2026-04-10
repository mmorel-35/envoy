#pragma once

// This file has moved to
// source/extensions/common/opentelemetry/exporters/otlp/http_trace_exporter.h It is kept here as a
// forwarding header for backward compatibility.

#include "source/extensions/common/opentelemetry/exporters/otlp/http_trace_exporter.h"
#include "source/extensions/tracers/opentelemetry/trace_exporter.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

// Re-export into the Tracers::OpenTelemetry namespace for backward compatibility.
using OpenTelemetryHttpTraceExporter =
    ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OtlpHttpTraceExporter;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

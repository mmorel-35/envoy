#pragma once

// Compatibility header - the HTTP metrics exporter now lives in shared OTLP.
#include "source/extensions/common/opentelemetry/exporters/otlp/http_metrics_exporter.h"

namespace Envoy {
namespace Extensions {
namespace StatSinks {
namespace OpenTelemetry {

// Backward-compatible alias for the shared OTLP HTTP metrics exporter.
using OpenTelemetryHttpMetricsExporter =
    ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OtlpHttpMetricsExporter;

} // namespace OpenTelemetry
} // namespace StatSinks
} // namespace Extensions
} // namespace Envoy

# OpenTelemetry Common Extensions

This directory provides shared types, constants, and utilities for OpenTelemetry extensions
in Envoy, used across tracers, access loggers, and stat sinks.

## Libraries

- **`otlp_types_lib`**: Common OpenTelemetry type aliases (`OTelAttribute`,
  `OtelAttributes`, `OTelSpanKind`, `KeyValue`, `AnyValue`).

- **`exporters/otlp:populate_attribute_utils_lib`**: Populates OTLP protobuf
  attribute values (`PopulateAttributeUtils::populateAnyValue`,
  `PopulateAttributeUtils::makeKeyValue`) in the
  `Envoy::Extensions::OpenTelemetry::Exporters::Otlp` namespace.
  Mirrors `opentelemetry::exporter::otlp::OtlpPopulateAttributeUtils` from opentelemetry-cpp.

- **`exporters/otlp:environment_lib`**: OTLP exporter environment helpers,
  including `Exporters::Otlp::GetUserAgent()` for the OTLP User-Agent header.

- **`sdk/logs:sdk_logs_constants_lib`**: OTLP logs signal constants
  (`Sdk::Logs::Constants::kLogsServiceExportMethod`, `kDefaultOtlpLogsEndpoint`).

- **`sdk/metrics:sdk_metrics_constants_lib`**: OTLP metrics signal constants
  (`Sdk::Metrics::Constants::kMetricsServiceExportMethod`, `kDefaultOtlpMetricsEndpoint`).

- **`sdk/trace:sdk_trace_constants_lib`**: OTLP trace signal constants
  (`Sdk::Trace::Constants::kTraceServiceExportMethod`, `kDefaultOtlpTracesEndpoint`).

## Namespace

Core types live in `Envoy::Extensions::OpenTelemetry`. Exporter utilities live
in the sub-namespace `Envoy::Extensions::OpenTelemetry::Exporters::Otlp`.
Signal-specific constants live in `Envoy::Extensions::Common::OpenTelemetry::Sdk::{Logs,Metrics,Trace}`.

## Usage

Extensions that export OTLP data should depend on these libraries instead of
duplicating constants or utility logic.

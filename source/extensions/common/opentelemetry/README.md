# OpenTelemetry Common Extensions

This directory provides shared types and utilities for OpenTelemetry extensions
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

## Namespace

Core types live in `Envoy::Extensions::OpenTelemetry`. Exporter utilities live
in the sub-namespace `Envoy::Extensions::OpenTelemetry::Exporters::Otlp`.

## Usage

Extensions that export OTLP data should depend on these libraries instead of
duplicating constants or utility logic.

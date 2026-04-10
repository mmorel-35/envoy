# OpenTelemetry Common Extensions

This directory provides shared types and utilities for OpenTelemetry extensions
in Envoy, used across tracers, access loggers, and stat sinks.

## Libraries

- **`otlp_types_lib`**: Common OpenTelemetry type aliases (`OTelAttribute`,
  `OtelAttributes`, `OTelSpanKind`, `KeyValue`, `AnyValue`).

- **`otlp_utils_lib`**: Shared OTLP utility functions (`getOtlpUserAgentHeader`,
  `populateAnyValue`, `getStringKeyValue`).

## Namespace

All code lives in `Envoy::Extensions::OpenTelemetry`.

## Usage

Extensions that export OTLP data should depend on these libraries instead of
duplicating constants or utility logic.

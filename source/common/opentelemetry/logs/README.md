# OpenTelemetry Logs

This directory contains logs-specific OpenTelemetry constants, types, and utilities used by log exporters in Envoy.

## Contents

- `types.h` - Log-specific type definitions including:
  - LogsExportRequest/Response types
  - Log-specific KeyValue types
  - Smart pointer aliases for log export operations

- `constants.h` - Log service constants including:
  - OTLP logs service method names
  - Standard endpoint paths for log export

## Usage

```cpp
#include "source/common/opentelemetry/logs/types.h"
#include "source/common/opentelemetry/logs/constants.h"

using namespace Envoy::Common::OpenTelemetry::Logs;

// Create a log export request
auto request = std::make_unique<LogsExportRequest>();

// Use service constants
const auto& method = LOGS_SERVICE_EXPORT_METHOD;
```

This code is specifically designed for OpenTelemetry log signal implementations and should not be used for other telemetry signals.
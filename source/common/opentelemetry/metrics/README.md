# OpenTelemetry Metrics

This directory contains OpenTelemetry metrics-specific constants, types, and utilities.

## Overview

The `source/common/opentelemetry/metrics/` directory provides metrics-specific OpenTelemetry functionality:

- **Metrics types** - Types for working with OpenTelemetry metrics
- **Aggregation temporality** - Temporal aggregation behaviors for metrics
- **Metrics export requests** - Request/response types for OTLP metrics exports
- **Metrics service constants** - Service method names and endpoint paths

## Files

- **`types.h`** - Metrics-specific type aliases and structures
  - `AggregationTemporality` - Enum for cumulative vs delta aggregation
  - `MetricsExportRequest` and `MetricsExportResponse` - OTLP export types
  - `KeyValue` - Common key-value pair type for metrics
  - Smart pointer aliases for export request types

- **`constants.h`** - Metrics service constants
  - `METRICS_SERVICE_EXPORT_METHOD` - gRPC service method for metrics export
  - `DEFAULT_OTLP_METRICS_ENDPOINT` - Default HTTP endpoint for metrics export

- **`BUILD`** - Bazel build configuration

## Usage

To use metrics-specific OpenTelemetry functionality in your code:

```cpp
#include "source/common/opentelemetry/metrics/types.h"
#include "source/common/opentelemetry/metrics/constants.h"

using namespace Envoy::Common::OpenTelemetry::Metrics;

// Use metrics types
AggregationTemporality temporality = AggregationTemporality::AGGREGATION_TEMPORALITY_CUMULATIVE;
MetricsExportRequestPtr request = std::make_unique<MetricsExportRequest>();

// Use metrics constants
const auto& service_method = Constants::METRICS_SERVICE_EXPORT_METHOD;
```

## BUILD Dependencies

Add to your BUILD file:
```bazel
deps = [
    "//source/common/opentelemetry/metrics:metrics_lib",
    # ... other deps
]
```

## Related Components

- [Traces](../traces/) - OpenTelemetry trace functionality
- [Logs](../logs/) - OpenTelemetry logs functionality  
- [Collector](../collector/) - Shared OTLP utilities across all signals

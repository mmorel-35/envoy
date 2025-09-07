# OpenTelemetry Common Library

This directory contains shared OpenTelemetry code used across all telemetry signals (traces, metrics, logs) in Envoy.

## Overview

The `source/common/opentelemetry/` directory centralizes OpenTelemetry functionality that was previously duplicated across multiple extensions:
- `source/extensions/tracers/opentelemetry/`
- `source/extensions/stat_sinks/open_telemetry/`
- `source/extensions/access_loggers/open_telemetry/`

## Components

### Protocol Constants (`protocol_constants.h`)
Centralizes OTLP service method strings to eliminate duplication:
- `TRACE_SERVICE_EXPORT_METHOD` - for gRPC trace exports
- `METRICS_SERVICE_EXPORT_METHOD` - for gRPC metrics exports  
- `LOGS_SERVICE_EXPORT_METHOD` - for gRPC logs exports
- Default OTLP endpoint paths

### Common Types (`types.h`)
Provides unified type aliases for all OpenTelemetry protocols:
- `OTelSpanKind`, `OTelAttribute`, `OTelAttributes` - span and attribute types
- `KeyValue`, `AggregationTemporality` - common protocol types
- Export request/response types for all signals with smart pointer aliases
- Ensures consistency across all telemetry signal implementations

### OTLP Utilities (`otlp_utils.h/cc`)
Shared utility functions for OTLP protocol operations:
- `getOtlpUserAgentHeader()` - standardized User-Agent for all OTLP exporters
- `populateAnyValue()` - converts OpenTelemetry attributes to protobuf AnyValue

## Usage

Extensions should include the appropriate headers:

```cpp
// For protocol constants
#include "source/common/opentelemetry/protocol_constants.h"

// For common types  
#include "source/common/opentelemetry/types.h"

// For OTLP utilities
#include "source/common/opentelemetry/otlp_utils.h"
```

## BUILD Dependencies

Add to your BUILD file:
```bazel
deps = [
    "//source/common/opentelemetry:opentelemetry_common_lib",
    # ... other deps
]
```

## Backward Compatibility

Existing extensions maintain backward compatibility through compatibility shims in their original locations. The original headers now forward to the centralized implementations.

## Benefits

- **Reduced Code Duplication**: Single source of truth for OTLP constants and utilities
- **Improved Consistency**: Standardized behavior across all telemetry signals
- **Easier Maintenance**: Changes only need to be made in one location
- **Faster Development**: Shared infrastructure accelerates new OpenTelemetry features

## Migration Strategy

This is the first phase of a larger OpenTelemetry code mutualization effort. Future phases will further consolidate shared functionality while maintaining backward compatibility.
# OpenTelemetry Common Library

This directory contains shared OpenTelemetry code used across all telemetry signals (traces, metrics, logs) in Envoy.

## Overview

The `source/common/opentelemetry/` directory centralizes OpenTelemetry functionality that was previously duplicated across multiple extensions:
- `source/extensions/tracers/opentelemetry/`
- `source/extensions/stat_sinks/open_telemetry/`
- `source/extensions/access_loggers/open_telemetry/`

For detailed background and implementation strategy, see [OPENTELEMETRY_MUTUALIZATION_PLAN.md](./OPENTELEMETRY_MUTUALIZATION_PLAN.md).

## Directory Structure

This directory uses a flat structure with clear, descriptive filenames that indicate their purpose:

- **`protocol_constants.h`** - OTLP protocol constants (service methods, endpoints)
- **`types.h`** - Common type aliases for all OpenTelemetry protocols
- **`otlp_utils.h/cc`** - Shared utility functions for OTLP operations
- **`README.md`** - This overview and usage documentation
- **`OPENTELEMETRY_MUTUALIZATION_PLAN.md`** - Detailed implementation plan and background
- **`BUILD`** - Bazel build configuration

### Organization Rationale

A flat structure was chosen for this initial phase because:
- **Small Scale**: Only 6 files currently, easy to navigate
- **Clear Naming**: File names clearly indicate their purpose and scope
- **Logical Grouping**: Related functionality is grouped by file type (constants, types, utilities)
- **Future Extensibility**: Structure can evolve to subdirectories if needed as the codebase grows

Future phases may introduce subdirectories for:
- `constants/` - Protocol values, limits, default configurations
- `helpers/` - Utility functions, parsing helpers, conversion utilities  
- `types/` - Shared types, data structures, type aliases
- `adapters/` - SDK/Envoy interop, compatibility layers

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

### Implementation Progress

✅ **Phase 1 (Current)** - Centralized Constants and Types
- OTLP service method strings unified across all extensions  
- Common type aliases for traces, metrics, and logs
- Shared OTLP utilities moved from tracers to common location
- Backward compatibility maintained through compatibility shims

🔄 **Phase 2 (Planned)** - Shared Validation and Configuration  
- Common protocol validation functions
- Unified configuration patterns
- Shared error handling utilities

🔄 **Phase 3 (Future)** - Advanced Features
- Environment variable support (`OTEL_SERVICE_NAME`, `OTEL_PROPAGATORS`, etc.)
- Enhanced resource detection capabilities
- Additional protocol support and optimizations

See [OPENTELEMETRY_MUTUALIZATION_PLAN.md](./OPENTELEMETRY_MUTUALIZATION_PLAN.md) for complete details and implementation timeline.

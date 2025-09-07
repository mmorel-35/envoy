# OpenTelemetry Common Library

This directory contains shared OpenTelemetry code used across all telemetry signals (traces, metrics, logs) in Envoy.

## Overview

The `source/common/opentelemetry/` directory centralizes OpenTelemetry functionality that was previously duplicated across multiple extensions:
- `source/extensions/tracers/opentelemetry/`
- `source/extensions/stat_sinks/open_telemetry/`
- `source/extensions/access_loggers/open_telemetry/`

For detailed background and implementation strategy, see [OPENTELEMETRY_MUTUALIZATION_PLAN.md](./OPENTELEMETRY_MUTUALIZATION_PLAN.md).

## Directory Structure

This directory is now organized by OpenTelemetry signal to provide clear separation and easier navigation:

### Signal-Specific Directories

- **[`traces/`](traces/)** - Trace-specific constants, types, and utilities
  - Span types and attributes (`OTelSpanKind`, `OTelAttribute`)
  - Trace export request/response types
  - Trace service constants and endpoints

- **[`metrics/`](metrics/)** - Metrics-specific constants, types, and utilities  
  - Metrics export request/response types
  - Aggregation temporality enums
  - Metrics service constants and endpoints

- **[`logs/`](logs/)** - Logs-specific constants, types, and utilities
  - Log export request/response types
  - Log service constants and endpoints

- **[`proto/`](proto/)** - Shared OTLP protocol utilities across all signals
  - Common OTLP protocol functions
  - User-Agent header standardization
  - Attribute conversion utilities

### Legacy Files (Deprecated)

For backward compatibility, the following files remain in the root directory but are deprecated:

- **`protocol_constants.h`** - Mixed signal constants (DEPRECATED)
- **`types.h`** - Mixed signal types (DEPRECATED)  
- **`otlp_utils.h/cc`** - Shared utilities (DEPRECATED, moved to `proto/`)

**Migration Path**: Use signal-specific headers instead:
```cpp
// Old (deprecated)
#include "source/common/opentelemetry/types.h"
#include "source/common/opentelemetry/protocol_constants.h"

// New (recommended)
#include "source/common/opentelemetry/traces/types.h"
#include "source/common/opentelemetry/traces/constants.h"
#include "source/common/opentelemetry/proto/otlp_utils.h"
```

## Organization Benefits

The signal-based organization provides:

- **Clear Signal Separation**: Each signal has its own dedicated namespace and files
- **Easier Navigation**: Developers can quickly find signal-specific functionality
- **Reduced Complexity**: Smaller, focused files instead of large mixed-signal files
- **Better Maintainability**: Changes to one signal don't affect others
- **Standard Compliance**: Follows OpenTelemetry C++ library organization patterns

## Components by Signal

### Traces (`traces/`)
- **Types**: `OTelSpanKind`, `OTelAttribute`, `TraceExportRequest`
- **Constants**: `TRACE_SERVICE_EXPORT_METHOD`, trace endpoints
- **Usage**: Span creation, trace export, trace context handling

### Metrics (`metrics/`)
- **Types**: `AggregationTemporality`, `MetricsExportRequest` 
- **Constants**: `METRICS_SERVICE_EXPORT_METHOD`, metrics endpoints
- **Usage**: Metrics collection, metrics export, stat sink implementations

### Logs (`logs/`)
- **Types**: `LogsExportRequest`, log-specific key-value types
- **Constants**: `LOGS_SERVICE_EXPORT_METHOD`, logs endpoints  
- **Usage**: Log export, access logger implementations

### Proto (`proto/`)
- **Utilities**: `getOtlpUserAgentHeader()`, `populateAnyValue()`
- **Cross-Signal**: Functions used by all telemetry signals
- **Usage**: OTLP protocol operations, attribute handling

## Usage

### Signal-Specific Usage

Include only the signal you need:

```cpp
// For traces only
#include "source/common/opentelemetry/traces/types.h"
#include "source/common/opentelemetry/traces/constants.h"

// For metrics only  
#include "source/common/opentelemetry/metrics/types.h"
#include "source/common/opentelemetry/metrics/constants.h"

// For cross-signal utilities
#include "source/common/opentelemetry/proto/otlp_utils.h"
```

### BUILD Dependencies

Use signal-specific dependencies:

```bazel
deps = [
    # For traces
    "//source/common/opentelemetry/traces:traces_lib",
    
    # For metrics
    "//source/common/opentelemetry/metrics:metrics_lib",
    
    # For logs
    "//source/common/opentelemetry/logs:logs_lib",
    
    # For shared utilities
    "//source/common/opentelemetry/proto:proto_lib",
    
    # Or everything (includes backward compatibility)
    "//source/common/opentelemetry:opentelemetry_common_lib",
]
```

## Backward Compatibility

Existing extensions maintain backward compatibility through the deprecated files in the root directory. These files now forward to the new signal-specific implementations where possible.

**Timeline**: 
- **Current**: Both old and new APIs are available
- **Next Release**: Deprecation warnings for old APIs
- **Future Release**: Removal of deprecated files

## Benefits

- **Reduced Code Duplication**: Signal-specific organization eliminates cross-signal contamination
- **Improved Consistency**: Each signal follows consistent patterns within its domain
- **Easier Maintenance**: Changes are scoped to specific signals
- **Faster Development**: Clear structure accelerates new OpenTelemetry features
- **Better Documentation**: Signal-specific documentation is more focused and useful

## Migration Strategy

This refactoring maintains full backward compatibility while introducing the new signal-based organization. Extensions can migrate at their own pace using the new structure.

### Implementation Progress

✅ **Phase 1 (Current)** - Signal-Based Directory Structure
- Trace-specific constants and types in `traces/`
- Metrics-specific constants and types in `metrics/`
- Logs-specific constants and types in `logs/`
- Shared utilities in `proto/`
- Backward compatibility maintained through root-level files

🔄 **Phase 2 (Next)** - Extension Migration
- Update extension includes to use signal-specific headers
- Add deprecation warnings to old APIs
- Update documentation and examples

🔄 **Phase 3 (Future)** - Cleanup
- Remove deprecated root-level files
- Complete migration to signal-based organization

See [OPENTELEMETRY_MUTUALIZATION_PLAN.md](./OPENTELEMETRY_MUTUALIZATION_PLAN.md) for complete details and implementation timeline.

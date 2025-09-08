# OpenTelemetry SDK for Envoy

This directory contains the OpenTelemetry SDK implementation for Envoy, organized following the official OpenTelemetry C++ SDK structure and conventions.

## Overview

The `source/extensions/opentelemetry/sdk/` directory provides SDK functionality that was previously located in `source/common/opentelemetry/`. This reorganization follows OpenTelemetry signal-based architecture and improves maintainability.

**Reference**: Inspired by [opentelemetry-cpp/sdk/src](https://github.com/open-telemetry/opentelemetry-cpp/tree/main/sdk/src)

## Directory Structure

The SDK is organized by OpenTelemetry signals and components, following official SDK conventions:

```
source/extensions/opentelemetry/sdk/
├── common/          # Shared types and constants used across all signals
├── configuration/   # SDK configuration and initialization (planned)
├── logs/           # Logs signal SDK implementation (planned)
├── metrics/        # Metrics signal SDK implementation (planned) 
├── resource/       # Resource detection and management (planned)
├── trace/          # Trace signal SDK implementation (planned)
├── version/        # Version information and utilities (planned)
├── BUILD           # Main SDK build configuration
└── README.md       # This documentation
```

### Current Implementation

#### `common/` - Shared SDK Components

Contains core types and constants used across all telemetry signals:

- **`protocol_constants.h`** - OTLP protocol constants organized by signal
  - Trace signal: gRPC service methods and HTTP endpoints
  - Metrics signal: gRPC service methods and HTTP endpoints  
  - Logs signal: gRPC service methods and HTTP endpoints
  - All constants derived from official OTLP specification

- **`types.h`** - Common type aliases organized by signal
  - Common types: Attributes, KeyValue pairs (from OpenTelemetry C++ SDK)
  - Trace signal: SpanKind, export request/response types (from OTLP spec)
  - Metrics signal: AggregationTemporality, export request/response types (from OTLP spec)
  - Logs signal: Export request/response types (from OTLP spec)
  - Smart pointer convenience aliases (Envoy extensions)

- **`BUILD`** - Build configuration with proper dependencies

### Future Implementation (Planned)

#### `trace/` - Trace Signal SDK
- Span processors and exporters
- Trace-specific configuration
- Sampling strategies

#### `metrics/` - Metrics Signal SDK  
- Metric readers and exporters
- Aggregation strategies
- Metrics-specific configuration

#### `logs/` - Logs Signal SDK
- Log record processors and exporters
- Logs-specific configuration

#### `resource/` - Resource Detection
- Resource detectors (environment, process, host)
- Resource merging and validation
- Container and cloud platform detection

#### `configuration/` - SDK Configuration
- Environment variable support (`OTEL_*` variables)
- Configuration validation and parsing
- Provider initialization

#### `version/` - Version Management
- SDK version information
- Compatibility tracking
- Feature flag management

## Annotations and Origin

### Signal Organization
All code is organized by OpenTelemetry signal type:
- **Trace**: Distributed tracing functionality
- **Metrics**: Application and infrastructure metrics  
- **Logs**: Structured logging with correlation

### Origin Classification
Code is annotated by origin:
- **OpenTelemetry C++ SDK**: Derived from official SDK implementation
- **OTLP Specification**: Derived from OpenTelemetry Protocol specification
- **Envoy Extensions**: Envoy-specific convenience and integration code

### Reference Links
- [OpenTelemetry Specification](https://github.com/open-telemetry/opentelemetry-specification)
- [OpenTelemetry C++ SDK](https://github.com/open-telemetry/opentelemetry-cpp)
- [OTLP Protocol](https://github.com/open-telemetry/opentelemetry-specification/blob/main/specification/protocol/otlp.md)

## Usage

### Including SDK Components

```cpp
// For protocol constants
#include "source/extensions/opentelemetry/sdk/common/protocol_constants.h"

// For common types  
#include "source/extensions/opentelemetry/sdk/common/types.h"
```

### BUILD Dependencies

```bazel
deps = [
    "//source/extensions/opentelemetry/sdk:opentelemetry_sdk_lib",
    # Or for specific components:
    "//source/extensions/opentelemetry/sdk/common:sdk_common_lib",
]
```

### Namespace Usage

```cpp
using namespace Envoy::Extensions::OpenTelemetry::Sdk::Common;

// Access protocol constants
auto trace_method = ProtocolConstants::TRACE_SERVICE_EXPORT_METHOD;
auto trace_endpoint = ProtocolConstants::DEFAULT_OTLP_TRACES_ENDPOINT;

// Use type aliases
TraceExportRequestPtr request = std::make_unique<TraceExportRequest>();
OTelAttributes attributes;
```

## Migration from Common

This SDK structure replaces `source/common/opentelemetry/` with several improvements:

### Before (Common)
```
source/common/opentelemetry/
├── protocol_constants.h  # Mixed signal constants
├── types.h              # Mixed signal types
├── otlp_utils.h/cc      # Mixed OTLP utilities
└── BUILD                # Single build target
```

### After (SDK)
```
source/extensions/opentelemetry/sdk/
├── common/
│   ├── protocol_constants.h  # Signal-organized constants with annotations
│   ├── types.h               # Signal-organized types with annotations
│   └── BUILD                 # Focused build targets
└── README.md                 # Comprehensive documentation
```

### Key Improvements
1. **Signal-based organization**: Clear separation by telemetry signal
2. **Origin annotations**: Code clearly marked by specification origin
3. **SDK conventions**: Follows official OpenTelemetry C++ SDK structure
4. **Enhanced documentation**: Comprehensive usage and reference documentation
5. **Modular build**: Granular build targets for better dependency management

## Relationship to Exporters

OTLP exporter functionality is kept separate in `source/extensions/opentelemetry/exporters/otlp/`:

- **SDK (this directory)**: Core OpenTelemetry types, constants, and SDK functionality
- **Exporters**: Protocol-specific export implementations (gRPC, HTTP)

This separation follows OpenTelemetry architecture where SDK provides core functionality and exporters provide protocol-specific implementations.

## Limited Scope

This implementation focuses on essential SDK components used by existing Envoy OpenTelemetry integrations. Future expansions will add additional SDK functionality as needed while maintaining this signal-based organization.

## Backward Compatibility

Existing code using `source/common/opentelemetry/` should be updated to use the new SDK structure. The namespace has changed from:

```cpp
// Old namespace
Envoy::Common::OpenTelemetry

// New namespace  
Envoy::Extensions::OpenTelemetry::Sdk::Common
```

Build targets have changed from:
```bazel
# Old target
"//source/common/opentelemetry:opentelemetry_common_lib"

# New target
"//source/extensions/opentelemetry/sdk:opentelemetry_sdk_lib"
```
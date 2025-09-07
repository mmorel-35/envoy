# OpenTelemetry Traces

This directory contains OpenTelemetry trace-specific constants, types, and utilities.

## Overview

The `source/common/opentelemetry/traces/` directory provides trace-specific OpenTelemetry functionality:

- **Span types and attributes** - Types for working with OpenTelemetry spans
- **Trace export requests** - Request/response types for OTLP trace exports
- **Trace service constants** - Service method names and endpoint paths

## Files

- **`types.h`** - Trace-specific type aliases and structures
  - `OTelSpanKind` - Enumeration of span kinds (internal, server, client, etc.)
  - `OTelAttribute` and `OTelAttributes` - Span attribute types
  - `TraceExportRequest` and `TraceExportResponse` - OTLP export types
  - Smart pointer aliases for export request types

- **`constants.h`** - Trace service constants
  - `TRACE_SERVICE_EXPORT_METHOD` - gRPC service method for trace export
  - `DEFAULT_OTLP_TRACES_ENDPOINT` - Default HTTP endpoint for trace export

- **`BUILD`** - Bazel build configuration

## Usage

To use trace-specific OpenTelemetry functionality in your code:

```cpp
#include "source/common/opentelemetry/traces/types.h"
#include "source/common/opentelemetry/traces/constants.h"

using namespace Envoy::Common::OpenTelemetry::Traces;

// Use trace types
OTelSpanKind span_kind = OTelSpanKind::SPAN_KIND_CLIENT;
TraceExportRequestPtr request = std::make_unique<TraceExportRequest>();

// Use trace constants
const auto& service_method = Constants::TRACE_SERVICE_EXPORT_METHOD;
```

## BUILD Dependencies

Add to your BUILD file:
```bazel
deps = [
    "//source/common/opentelemetry/traces:traces_lib",
    # ... other deps
]
```

## Related Components

- [Metrics](../metrics/) - OpenTelemetry metrics functionality
- [Logs](../logs/) - OpenTelemetry logs functionality  
- [Collector](../collector/) - Shared OTLP utilities across all signals

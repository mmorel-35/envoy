# OpenTelemetry Collector

This directory contains shared OpenTelemetry collector utilities used across all telemetry signals.

## Overview

The `source/common/opentelemetry/collector/` directory provides collector-specific OpenTelemetry functionality that is shared across traces, metrics, and logs:

- **OTLP utilities** - Common functions for OTLP protocol operations
- **User-Agent headers** - Standardized headers for OTLP exporters
- **Attribute handling** - Functions to convert OpenTelemetry attributes to protobuf

## Files

- **`otlp_utils.h/cc`** - Shared OTLP utility functions
  - `getOtlpUserAgentHeader()` - Standard User-Agent header for all OTLP exporters
  - `populateAnyValue()` - Convert OpenTelemetry attributes to protobuf AnyValue

- **`BUILD`** - Bazel build configuration

## Usage

To use collector OpenTelemetry functionality in your code:

```cpp
#include "source/common/opentelemetry/collector/otlp_utils.h"
#include "source/common/opentelemetry/traces/types.h"  // For OTelAttribute

using namespace Envoy::Common::OpenTelemetry;

// Get standard User-Agent header
const auto& user_agent = Collector::OtlpUtils::getOtlpUserAgentHeader();

// Convert attribute to protobuf
Traces::OTelAttribute attribute = std::string("example_value");
opentelemetry::proto::common::v1::AnyValue proto_value;
Collector::OtlpUtils::populateAnyValue(proto_value, attribute);
```

## BUILD Dependencies

Add to your BUILD file:
```bazel
deps = [
    "//source/common/opentelemetry/collector:collector_lib",
    # ... other deps
]
```

## Features

### Standard User-Agent Header

The `getOtlpUserAgentHeader()` function provides a compliant User-Agent header that follows the OpenTelemetry specification:

Format: `OTel-OTLP-Exporter-Envoy/{version}`

This ensures consistent identification of Envoy as the OTLP exporter across all telemetry signals.

### Attribute Conversion

The `populateAnyValue()` function handles conversion of OpenTelemetry attributes to protobuf AnyValue messages, supporting all standard attribute types:

- Boolean values
- Integer values (int32, int64, uint32, uint64)
- Double precision floating point
- String values (C-string and string_view)

## Related Components

- [Traces](../traces/) - OpenTelemetry trace functionality
- [Metrics](../metrics/) - OpenTelemetry metrics functionality  
- [Logs](../logs/) - OpenTelemetry logs functionality
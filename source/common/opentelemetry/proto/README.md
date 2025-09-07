# OpenTelemetry Protocol Utilities

This directory contains utilities for working with the OpenTelemetry Protocol (OTLP) that are shared across all telemetry signals (traces, metrics, logs).

## Contents

- `otlp_utils.h/cc` - Core OTLP protocol utilities including:
  - User-Agent header generation for OTLP exporters
  - Protocol buffer serialization helpers
  - Common attribute handling functions

## Usage

```cpp
#include "source/common/opentelemetry/proto/otlp_utils.h"

// Get the OTLP user agent header
const auto& user_agent = Envoy::Common::OpenTelemetry::Proto::OtlpUtils::getOtlpUserAgentHeader();

// Populate a protocol buffer with an attribute value
opentelemetry::proto::common::v1::AnyValue proto_value;
Envoy::Common::OpenTelemetry::Proto::OtlpUtils::populateAnyValue(proto_value, attribute);
```

These utilities handle the low-level protocol details and wire format conversions needed by all OpenTelemetry signal implementations in Envoy.

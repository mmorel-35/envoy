#pragma once

// Forwarding header for backward compatibility.
// New code should include:
//   source/extensions/common/opentelemetry/populate_attribute_utils.h
//   source/extensions/common/opentelemetry/exporters/otlp/environment.h
#include "source/extensions/common/opentelemetry/exporters/otlp/environment.h"
#include "source/extensions/common/opentelemetry/populate_attribute_utils.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {

// Backward-compatibility alias so existing code that refers to OtlpUtils compiles unchanged.
using OtlpUtils = PopulateAttributeUtils;

} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

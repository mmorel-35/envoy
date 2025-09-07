#pragma once

// This header is deprecated. Use source/common/opentelemetry/ instead.
// Compatibility shim for existing code.

#include "source/common/opentelemetry/otlp_utils.h"
#include "source/common/opentelemetry/types.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

// Type aliases for backward compatibility
using OTelSpanKind = ::Envoy::Common::OpenTelemetry::OTelSpanKind;
using OTelAttribute = ::Envoy::Common::OpenTelemetry::OTelAttribute;
using OtelAttributes = ::Envoy::Common::OpenTelemetry::OTelAttributes;

// Utility class for backward compatibility
using OtlpUtils = ::Envoy::Common::OpenTelemetry::OtlpUtils;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

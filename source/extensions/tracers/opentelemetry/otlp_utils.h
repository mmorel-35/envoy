#pragma once

// This header is deprecated. Use source/common/opentelemetry/ instead.
// Compatibility shim for existing code.

#include "source/common/opentelemetry/proto/otlp_utils.h"
#include "source/common/opentelemetry/traces/types.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

// Type aliases for backward compatibility
using OTelSpanKind = ::Envoy::Common::OpenTelemetry::Traces::OTelSpanKind;
using OTelAttribute = ::Envoy::Common::OpenTelemetry::Traces::OTelAttribute;
using OtelAttributes = ::Envoy::Common::OpenTelemetry::Traces::OTelAttributes;

// Utility class for backward compatibility
using OtlpUtils = ::Envoy::Common::OpenTelemetry::Proto::OtlpUtils;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

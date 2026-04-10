#pragma once

#include "source/extensions/common/opentelemetry/otlp_utils.h"
#include "source/extensions/common/opentelemetry/types.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

// Import shared types into the tracer namespace for backward compatibility.
using OTelSpanKind = ::Envoy::Extensions::OpenTelemetry::OTelSpanKind;
using OTelAttribute = ::Envoy::Extensions::OpenTelemetry::OTelAttribute;
using OtelAttributes = ::Envoy::Extensions::OpenTelemetry::OtelAttributes;
using OtlpUtils = ::Envoy::Extensions::OpenTelemetry::OtlpUtils;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

#pragma once

#include "source/extensions/common/opentelemetry/populate_attribute_utils.h"
#include "source/extensions/common/opentelemetry/types.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

// Import shared types into the tracer namespace for backward compatibility.
using OTelSpanKind = ::Envoy::Extensions::OpenTelemetry::OTelSpanKind;
using OTelAttribute = ::Envoy::Extensions::OpenTelemetry::OTelAttribute;
using OtelAttributes = ::Envoy::Extensions::OpenTelemetry::OtelAttributes;
// OtlpUtils is kept as an alias so existing tracer code compiles unchanged.
using OtlpUtils = ::Envoy::Extensions::OpenTelemetry::PopulateAttributeUtils;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

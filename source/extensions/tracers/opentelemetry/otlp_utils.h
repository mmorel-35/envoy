#pragma once

// This header is deprecated. Use source/extensions/opentelemetry/ instead.
// Compatibility shim for existing code.

#include "source/extensions/opentelemetry/exporters/otlp/otlp_utils.h"
#include "source/extensions/opentelemetry/sdk/common/types.h"
#include "source/extensions/opentelemetry/sdk/version/version.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

// Type aliases for backward compatibility
using OTelSpanKind = ::Envoy::Extensions::OpenTelemetry::Sdk::Common::OTelSpanKind;
using OTelAttribute = ::Envoy::Extensions::OpenTelemetry::Sdk::Common::OTelAttribute;
using OtelAttributes = ::Envoy::Extensions::OpenTelemetry::Sdk::Common::OTelAttributes;

// Utility class for backward compatibility
class OtlpUtils {
public:
  static const std::string& getOtlpUserAgentHeader() {
    return ::Envoy::Extensions::OpenTelemetry::Sdk::Version::VersionUtils::getOtlpUserAgentHeader();
  }

  static void populateAnyValue(opentelemetry::proto::common::v1::AnyValue& value_proto,
                               const OTelAttribute& attribute_value) {
    ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OtlpUtils::populateAnyValue(value_proto, attribute_value);
  }
};

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

#pragma once

#include <string>

#include "source/extensions/common/opentelemetry/types.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

class PopulateAttributeUtils {
public:
  static void populateAnyValue(AnyValue& value_proto, const OTelAttribute& attribute_value);
  static KeyValue makeKeyValue(const std::string& key, const std::string& value);
};

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

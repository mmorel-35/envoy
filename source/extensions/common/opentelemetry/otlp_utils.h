#pragma once

#include <string>

#include "source/extensions/common/opentelemetry/types.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {

/**
 * Contains utility functions for OTLP exporters shared across OpenTelemetry
 * extensions (tracers, access loggers, stat sinks).
 */
class OtlpUtils {
public:
  /**
   * @brief Get the User-Agent header value to be used on the OTLP exporter request.
   *
   * The header value is compliant with the OpenTelemetry specification. See:
   * https://github.com/open-telemetry/opentelemetry-specification/blob/v1.30.0/specification/protocol/exporter.md#user-agent
   * @return std::string The User-Agent for the OTLP exporters in Envoy.
   */
  static const std::string& getOtlpUserAgentHeader();

  /**
   * @brief Set the Otel attribute on a Proto Value object.
   *
   * @param value_proto Proto object which gets the value set.
   * @param attribute_value Value to set on the proto object.
   */
  static void populateAnyValue(AnyValue& value_proto, const OTelAttribute& attribute_value);

  /**
   * @brief Create a KeyValue protobuf with a string value.
   *
   * @param key The key string.
   * @param value The value string.
   * @return KeyValue A KeyValue proto with the given key and string value.
   */
  static KeyValue getStringKeyValue(const std::string& key, const std::string& value);
};

} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

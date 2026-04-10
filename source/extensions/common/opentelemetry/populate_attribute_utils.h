#pragma once

#include <string>

#include "source/extensions/common/opentelemetry/types.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {

/**
 * Utility functions for populating OTLP protobuf attribute types.
 * Shared across OpenTelemetry extensions (tracers, access loggers, stat sinks).
 *
 * Mirrors opentelemetry::exporter::otlp::OtlpPopulateAttributeUtils from opentelemetry-cpp.
 * @see https://github.com/open-telemetry/opentelemetry-cpp/blob/main/exporters/otlp/include/opentelemetry/exporters/otlp/otlp_populate_attribute_utils.h
 */
class PopulateAttributeUtils {
public:
  /**
   * @brief Populate an AnyValue proto from an attribute value variant.
   *
   * @param value_proto Proto object to populate.
   * @param attribute_value Value to set on the proto object.
   */
  static void populateAnyValue(AnyValue& value_proto,
                               const Sdk::Common::AttributeValue& attribute_value);

  /**
   * @brief Create a KeyValue protobuf with a string value.
   *
   * @param key The key string.
   * @param value The value string.
   * @return KeyValue A KeyValue proto with the given key and string value.
   */
  static KeyValue makeKeyValue(const std::string& key, const std::string& value);
};

} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

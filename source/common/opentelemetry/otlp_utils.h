#pragma once

#include <string>

#include "source/common/opentelemetry/types.h"

#include "opentelemetry/proto/common/v1/common.pb.h"

namespace Envoy {
namespace Common {
namespace OpenTelemetry {

/**
 * Contains utility functions for OpenTelemetry OTLP protocol operations.
 * These utilities are shared across all telemetry signals (traces, metrics, logs).
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
   * @brief Set the OpenTelemetry attribute on a Proto AnyValue object
   *
   * @param value_proto Proto object which gets the value set.
   * @param attribute_value Value to set on the proto object.
   */
  static void populateAnyValue(opentelemetry::proto::common::v1::AnyValue& value_proto,
                               const OTelAttribute& attribute_value);
};

} // namespace OpenTelemetry
} // namespace Common
} // namespace Envoy

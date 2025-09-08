#pragma once

#include <string>

#include "source/common/opentelemetry/types.h"

#include "opentelemetry/proto/common/v1/common.pb.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

/**
 * Contains utility functions for OpenTelemetry OTLP protocol operations.
 * These utilities are shared across all telemetry signals (traces, metrics, logs).
 */
class OtlpUtils {
public:
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
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

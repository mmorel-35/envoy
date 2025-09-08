#pragma once

#include <string>

#include "source/extensions/opentelemetry/sdk/common/types.h"

#include "absl/strings/string_view.h"
#include "opentelemetry/proto/common/v1/common.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

/**
 * Constants for OpenTelemetry OTLP service methods used across all telemetry signals.
 * These constants ensure consistency and reduce duplication across tracers, stat sinks,
 * and access loggers.
 */
class ProtocolConstants {
public:
  // OTLP service method names for gRPC exporters
  static constexpr absl::string_view TRACE_SERVICE_EXPORT_METHOD =
      "opentelemetry.proto.collector.trace.v1.TraceService.Export";

  static constexpr absl::string_view METRICS_SERVICE_EXPORT_METHOD =
      "opentelemetry.proto.collector.metrics.v1.MetricsService.Export";

  static constexpr absl::string_view LOGS_SERVICE_EXPORT_METHOD =
      "opentelemetry.proto.collector.logs.v1.LogsService.Export";

  // Default OTLP endpoints
  static constexpr absl::string_view DEFAULT_OTLP_TRACES_ENDPOINT = "/v1/traces";
  static constexpr absl::string_view DEFAULT_OTLP_METRICS_ENDPOINT = "/v1/metrics";
  static constexpr absl::string_view DEFAULT_OTLP_LOGS_ENDPOINT = "/v1/logs";
};

/**
 * Utility functions for OTLP protocol operations.
 * These utilities are used by OTLP exporters for different telemetry signals.
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
                               const Sdk::Common::OTelAttribute& attribute_value);
};

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy
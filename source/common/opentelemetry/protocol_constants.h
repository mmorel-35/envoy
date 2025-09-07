#pragma once

#include <string>

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Common {
namespace OpenTelemetry {

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

} // namespace OpenTelemetry
} // namespace Common
} // namespace Envoy

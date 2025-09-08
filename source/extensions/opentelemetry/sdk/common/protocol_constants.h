#pragma once

#include <string>

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Sdk {
namespace Common {

/**
 * Constants for OpenTelemetry OTLP service methods used across all telemetry signals.
 * These constants ensure consistency and reduce duplication across tracers, stat sinks,
 * and access loggers.
 * 
 * Organization: Constants are grouped by OpenTelemetry signal type (trace, metrics, logs)
 * Origin: All constants are derived from official OpenTelemetry Protocol (OTLP) specification
 * Reference: https://github.com/open-telemetry/opentelemetry-specification/blob/main/specification/protocol/otlp.md
 */
class ProtocolConstants {
public:
  // === TRACE SIGNAL ===
  // OpenTelemetry Protocol constants for trace telemetry signal
  
  // gRPC service method for trace exports (from OTLP spec)
  static constexpr absl::string_view TRACE_SERVICE_EXPORT_METHOD =
      "opentelemetry.proto.collector.trace.v1.TraceService.Export";
  
  // HTTP endpoint for trace exports (from OTLP spec)
  static constexpr absl::string_view DEFAULT_OTLP_TRACES_ENDPOINT = "/v1/traces";

  // === METRICS SIGNAL ===
  // OpenTelemetry Protocol constants for metrics telemetry signal
  
  // gRPC service method for metrics exports (from OTLP spec)
  static constexpr absl::string_view METRICS_SERVICE_EXPORT_METHOD =
      "opentelemetry.proto.collector.metrics.v1.MetricsService.Export";
  
  // HTTP endpoint for metrics exports (from OTLP spec)
  static constexpr absl::string_view DEFAULT_OTLP_METRICS_ENDPOINT = "/v1/metrics";

  // === LOGS SIGNAL ===
  // OpenTelemetry Protocol constants for logs telemetry signal
  
  // gRPC service method for logs exports (from OTLP spec)
  static constexpr absl::string_view LOGS_SERVICE_EXPORT_METHOD =
      "opentelemetry.proto.collector.logs.v1.LogsService.Export";
  
  // HTTP endpoint for logs exports (from OTLP spec)
  static constexpr absl::string_view DEFAULT_OTLP_LOGS_ENDPOINT = "/v1/logs";
};

} // namespace Common
} // namespace Sdk
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

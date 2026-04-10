#pragma once
// NOLINT(namespace-envoy)

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Common {
namespace OpenTelemetry {
namespace Sdk {
namespace Logs {

/**
 * Constants for OpenTelemetry OTLP logs signal.
 * These constants ensure consistency across log exporters and access loggers.
 *
 * Origin: All constants are derived from the official OpenTelemetry Protocol (OTLP) specification.
 * Reference:
 * https://github.com/open-telemetry/opentelemetry-specification/blob/main/specification/protocol/otlp.md
 */
struct Constants {
  // gRPC service method for log exports (from OTLP spec).
  static constexpr absl::string_view kLogsServiceExportMethod =
      "opentelemetry.proto.collector.logs.v1.LogsService.Export";

  // Default HTTP endpoint for log exports (from OTLP spec).
  static constexpr absl::string_view kDefaultOtlpLogsEndpoint = "/v1/logs";
};

} // namespace Logs
} // namespace Sdk
} // namespace OpenTelemetry
} // namespace Common
} // namespace Extensions
} // namespace Envoy

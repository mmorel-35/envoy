#pragma once
// NOLINT(namespace-envoy)

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Common {
namespace OpenTelemetry {
namespace Sdk {
namespace Metrics {

/**
 * Constants for OpenTelemetry OTLP metrics signal.
 * These constants ensure consistency across metrics exporters and stat sinks.
 *
 * Origin: All constants are derived from the official OpenTelemetry Protocol (OTLP) specification.
 * Reference:
 * https://github.com/open-telemetry/opentelemetry-specification/blob/main/specification/protocol/otlp.md
 */
struct Constants {
  // gRPC service method for metrics exports (from OTLP spec).
  static constexpr absl::string_view kMetricsServiceExportMethod =
      "opentelemetry.proto.collector.metrics.v1.MetricsService.Export";

  // Default HTTP endpoint for metrics exports (from OTLP spec).
  static constexpr absl::string_view kDefaultOtlpMetricsEndpoint = "/v1/metrics";
};

} // namespace Metrics
} // namespace Sdk
} // namespace OpenTelemetry
} // namespace Common
} // namespace Extensions
} // namespace Envoy

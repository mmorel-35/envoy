#pragma once

#include <string>

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Common {
namespace OpenTelemetry {
namespace Metrics {

/**
 * Constants for OpenTelemetry OTLP metrics service operations.
 */
class Constants {
public:
  // OTLP service method name for gRPC metrics exporters
  static constexpr absl::string_view METRICS_SERVICE_EXPORT_METHOD =
      "opentelemetry.proto.collector.metrics.v1.MetricsService.Export";

  // Default OTLP metrics endpoint
  static constexpr absl::string_view DEFAULT_OTLP_METRICS_ENDPOINT = "/v1/metrics";
};

} // namespace Metrics
} // namespace OpenTelemetry
} // namespace Common
} // namespace Envoy
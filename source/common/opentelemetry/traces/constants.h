#pragma once

#include <string>

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Common {
namespace OpenTelemetry {
namespace Traces {

/**
 * Constants for OpenTelemetry OTLP trace service operations.
 */
class Constants {
public:
  // OTLP service method name for gRPC trace exporters
  static constexpr absl::string_view TRACE_SERVICE_EXPORT_METHOD =
      "opentelemetry.proto.collector.trace.v1.TraceService.Export";

  // Default OTLP trace endpoint
  static constexpr absl::string_view DEFAULT_OTLP_TRACES_ENDPOINT = "/v1/traces";
};

} // namespace Traces
} // namespace OpenTelemetry
} // namespace Common
} // namespace Envoy
#pragma once
// NOLINT(namespace-envoy)

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Common {
namespace OpenTelemetry {
namespace Sdk {
namespace Trace {

/**
 * Constants for OpenTelemetry OTLP trace signal.
 * These constants ensure consistency across trace exporters and tracers.
 *
 * Origin: All constants are derived from the official OpenTelemetry Protocol (OTLP) specification.
 * Reference:
 * https://github.com/open-telemetry/opentelemetry-specification/blob/main/specification/protocol/otlp.md
 */
struct Constants {
  // gRPC service method for trace exports (from OTLP spec).
  static constexpr absl::string_view kTraceServiceExportMethod =
      "opentelemetry.proto.collector.trace.v1.TraceService.Export";

  // Default HTTP endpoint for trace exports (from OTLP spec).
  static constexpr absl::string_view kDefaultOtlpTracesEndpoint = "/v1/traces";
};

} // namespace Trace
} // namespace Sdk
} // namespace OpenTelemetry
} // namespace Common
} // namespace Extensions
} // namespace Envoy

#pragma once

#include <string>

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Common {
namespace OpenTelemetry {
namespace Logs {

// OTLP Log Service Constants
constexpr absl::string_view LOGS_SERVICE_EXPORT_METHOD = "opentelemetry.proto.collector.logs.v1.LogsService/Export";

// Common endpoint patterns for OTLP log services
constexpr absl::string_view LOGS_SERVICE_GRPC_PATH = "/opentelemetry.proto.collector.logs.v1.LogsService/Export";
constexpr absl::string_view LOGS_SERVICE_HTTP_PATH = "/v1/logs";

} // namespace Logs
} // namespace OpenTelemetry
} // namespace Common
} // namespace Envoy
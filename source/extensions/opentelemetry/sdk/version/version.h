#pragma once

#include <string>

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Sdk {
namespace Version {

/**
 * Version and user-agent utilities for OpenTelemetry SDK.
 */
class VersionUtils {
public:
  /**
   * @brief Get the User-Agent header value to be used on the OTLP exporter request.
   *
   * The header value is compliant with the OpenTelemetry specification. See:
   * https://github.com/open-telemetry/opentelemetry-specification/blob/v1.30.0/specification/protocol/exporter.md#user-agent
   * @return std::string The User-Agent for the OTLP exporters in Envoy.
   */
  static const std::string& getOtlpUserAgentHeader();
};

} // namespace Version
} // namespace Sdk
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy
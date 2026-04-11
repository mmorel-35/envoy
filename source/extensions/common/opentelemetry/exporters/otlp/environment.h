#pragma once

#include <string>

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

/**
 * @brief Get the User-Agent header value to be used on OTLP exporter requests.
 *
 * The value is compliant with the OpenTelemetry specification. See:
 * https://github.com/open-telemetry/opentelemetry-specification/blob/v1.30.0/specification/protocol/exporter.md#user-agent
 *
 * Mirrors `opentelemetry::exporter::otlp::GetOtlpDefaultUserAgent()` from `opentelemetry-cpp`.
 * @see
 * https://github.com/open-telemetry/opentelemetry-cpp/blob/main/exporters/otlp/include/opentelemetry/exporters/otlp/otlp_environment.h
 *
 * @return const std::string& The User-Agent for the OTLP exporters in Envoy.
 */
const std::string& GetUserAgent();

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

#include "source/extensions/opentelemetry/sdk/version/version.h"

#include <string>

#include "source/common/common/fmt.h"
#include "source/common/common/macros.h"
#include "source/common/version/version.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Sdk {
namespace Version {

const std::string& VersionUtils::getOtlpUserAgentHeader() {
  CONSTRUCT_ON_FIRST_USE(std::string,
                         fmt::format("OTel-OTLP-Exporter-Envoy/{}", Envoy::VersionInfo::version()));
}

} // namespace Version
} // namespace Sdk
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy
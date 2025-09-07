#include "source/extensions/stat_sinks/open_telemetry/open_telemetry_proto_descriptors.h"

#include "source/common/common/assert.h"
#include "source/common/opentelemetry/metrics/constants.h"
#include "source/common/protobuf/protobuf.h"

namespace Envoy {
namespace Extensions {
namespace StatSinks {
namespace OpenTelemetry {

void validateProtoDescriptors() {
  const auto method =
      std::string(Envoy::Common::OpenTelemetry::Metrics::Constants::METRICS_SERVICE_EXPORT_METHOD);

  RELEASE_ASSERT(Protobuf::DescriptorPool::generated_pool()->FindMethodByName(method) != nullptr,
                 "");
};

} // namespace OpenTelemetry
} // namespace StatSinks
} // namespace Extensions
} // namespace Envoy

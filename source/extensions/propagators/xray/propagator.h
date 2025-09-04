#pragma once

#include <string>

#include "envoy/http/header_map.h"

#include "source/common/singleton/const_singleton.h"
#include "source/common/tracing/trace_context_impl.h"
#include "source/extensions/propagators/xray/trace_context.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace XRay {

/**
 * AWS X-Ray trace propagation constants for header names as defined in:
 * https://docs.aws.amazon.com/xray/latest/devguide/xray-concepts.html#xray-concepts-tracingheader
 */
class XRayConstantValues {
public:
  // AWS X-Ray trace header
  const Tracing::TraceContextHandler X_AMZN_TRACE_ID{std::string(Constants::kTraceIdHeader)};
};

using XRayConstants = ConstSingleton<XRayConstantValues>;

} // namespace XRay
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
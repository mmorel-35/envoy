#pragma once

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace XRay {

/**
 * AWS X-Ray trace propagation specification constants.
 * See
 * https://docs.aws.amazon.com/xray/latest/devguide/xray-concepts.html#xray-concepts-tracingheader
 */
namespace Constants {
// AWS X-Ray trace header name
constexpr absl::string_view kTraceIdHeader = "x-amzn-trace-id";
} // namespace Constants

} // namespace XRay
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

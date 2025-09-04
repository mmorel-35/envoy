#pragma once

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace SkyWalking {

/**
 * SkyWalking trace propagation specification constants.
 * See
 * https://skywalking.apache.org/docs/main/latest/en/protocols/skywalking-cross-process-propagation-headers-protocol-v3/
 */
namespace Constants {
// SkyWalking trace header name
constexpr absl::string_view kSw8Header = "sw8";
} // namespace Constants

} // namespace SkyWalking
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

#pragma once

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace Propagation {
namespace B3 {

/**
 * B3 Trace Propagation specification constants.
 * See https://github.com/openzipkin/b3-propagation
 */
namespace Constants {
// B3 multi-header format header names
constexpr absl::string_view kTraceIdHeader = "x-b3-traceid";
constexpr absl::string_view kSpanIdHeader = "x-b3-spanid";
constexpr absl::string_view kParentSpanIdHeader = "x-b3-parentspanid";
constexpr absl::string_view kSampledHeader = "x-b3-sampled";
constexpr absl::string_view kFlagsHeader = "x-b3-flags";

// B3 single header format
constexpr absl::string_view kB3Header = "b3";
} // namespace Constants

} // namespace B3
} // namespace Propagation
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

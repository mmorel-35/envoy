#pragma once

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace B3 {

/**
 * B3 Trace Propagation specification constants.
 * See https://github.com/openzipkin/b3-propagation
 */
namespace Multi {
namespace Constants {
// B3 multi-header format header names
constexpr absl::string_view kTraceIdHeader = "x-b3-traceid";
constexpr absl::string_view kSpanIdHeader = "x-b3-spanid";
constexpr absl::string_view kParentSpanIdHeader = "x-b3-parentspanid";
constexpr absl::string_view kSampledHeader = "x-b3-sampled";
constexpr absl::string_view kFlagsHeader = "x-b3-flags";
} // namespace Constants
} // namespace Multi

namespace Single {
namespace Constants {
// B3 single header format
constexpr absl::string_view kB3Header = "b3";
} // namespace Constants
} // namespace Single

// Legacy namespace for backward compatibility
namespace Constants {
// B3 multi-header format header names
constexpr absl::string_view kTraceIdHeader = Multi::Constants::kTraceIdHeader;
constexpr absl::string_view kSpanIdHeader = Multi::Constants::kSpanIdHeader;
constexpr absl::string_view kParentSpanIdHeader = Multi::Constants::kParentSpanIdHeader;
constexpr absl::string_view kSampledHeader = Multi::Constants::kSampledHeader;
constexpr absl::string_view kFlagsHeader = Multi::Constants::kFlagsHeader;

// B3 single header format
constexpr absl::string_view kB3Header = Single::Constants::kB3Header;
} // namespace Constants

} // namespace B3
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

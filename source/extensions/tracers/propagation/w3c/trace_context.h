#pragma once

#include <cstdint>
#include <cstddef>

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace Propagation {
namespace W3c {

/**
 * W3C Trace Context specification constants.
 * See https://www.w3.org/TR/trace-context/
 */
namespace Constants {
// W3C traceparent header format: version-trace-id-parent-id-trace-flags
constexpr int kTraceparentHeaderSize = 55; // 2 + 1 + 32 + 1 + 16 + 1 + 2
constexpr int kVersionSize = 2;
constexpr int kTraceIdSize = 32;
constexpr int kParentIdSize = 16;
constexpr int kTraceFlagsSize = 2;

// Header names as defined in W3C specification
constexpr absl::string_view kTraceparentHeader = "traceparent";
constexpr absl::string_view kTracestateHeader = "tracestate";
constexpr absl::string_view kBaggageHeader = "baggage";

// Current version
constexpr absl::string_view kCurrentVersion = "00";

// Trace flags
constexpr uint8_t kSampledFlag = 0x01;

// W3C Baggage specification limits
// See https://www.w3.org/TR/baggage/
constexpr size_t kMaxBaggageSize = 8192;   // 8KB total size limit
constexpr size_t kMaxBaggageMembers = 180; // Practical limit to prevent abuse
constexpr size_t kMaxKeyLength = 256;
constexpr size_t kMaxValueLength = 4096;
} // namespace Constants

} // namespace W3c
} // namespace Propagation
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
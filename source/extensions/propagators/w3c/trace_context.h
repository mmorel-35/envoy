#pragma once

#include <cstddef>
#include <cstdint>

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3c {

namespace TraceContext {
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

// Current version
constexpr absl::string_view kCurrentVersion = "00";

// Trace flags
constexpr uint8_t kSampledFlag = 0x01;
} // namespace Constants
} // namespace TraceContext

namespace Baggage {
/**
 * W3C Baggage specification constants.
 * See https://www.w3.org/TR/baggage/
 */
namespace Constants {
// Header names as defined in W3C specification
constexpr absl::string_view kBaggageHeader = "baggage";

// W3C Baggage specification limits
constexpr size_t kMaxBaggageSize = 8192;   // 8KB total size limit
constexpr size_t kMaxBaggageMembers = 180; // Practical limit to prevent abuse
constexpr size_t kMaxKeyLength = 256;
constexpr size_t kMaxValueLength = 4096;
} // namespace Constants
} // namespace Baggage

// Legacy namespace for backward compatibility
namespace Constants {
// W3C traceparent header format: version-trace-id-parent-id-trace-flags
constexpr int kTraceparentHeaderSize = TraceContext::Constants::kTraceparentHeaderSize;
constexpr int kVersionSize = TraceContext::Constants::kVersionSize;
constexpr int kTraceIdSize = TraceContext::Constants::kTraceIdSize;
constexpr int kParentIdSize = TraceContext::Constants::kParentIdSize;
constexpr int kTraceFlagsSize = TraceContext::Constants::kTraceFlagsSize;

// Header names as defined in W3C specification
constexpr absl::string_view kTraceparentHeader = TraceContext::Constants::kTraceparentHeader;
constexpr absl::string_view kTracestateHeader = TraceContext::Constants::kTracestateHeader;
constexpr absl::string_view kBaggageHeader = Baggage::Constants::kBaggageHeader;

// Current version
constexpr absl::string_view kCurrentVersion = TraceContext::Constants::kCurrentVersion;

// Trace flags
constexpr uint8_t kSampledFlag = TraceContext::Constants::kSampledFlag;

// W3C Baggage specification limits
constexpr size_t kMaxBaggageSize = Baggage::Constants::kMaxBaggageSize;
constexpr size_t kMaxBaggageMembers = Baggage::Constants::kMaxBaggageMembers;
constexpr size_t kMaxKeyLength = Baggage::Constants::kMaxKeyLength;
constexpr size_t kMaxValueLength = Baggage::Constants::kMaxValueLength;
} // namespace Constants

} // namespace W3c
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

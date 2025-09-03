#include "source/extensions/propagators/b3/trace_context.h"

#include <cctype>
#include <charconv>
#include <iomanip>
#include <sstream>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "source/common/common/utility.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace B3 {

namespace {

/**
 * Validates that a string contains only valid hex characters.
 */
bool isValidHex(absl::string_view hex_string) {
  if (hex_string.empty()) {
    return false;
  }
  for (char c : hex_string) {
    if (!std::isxdigit(c)) {
      return false;
    }
  }
  return true;
}

/**
 * Converts hex string to uint64_t using std::from_chars for better performance.
 */
bool parseHexString(absl::string_view hex_string, uint64_t& result) {
  if (!isValidHex(hex_string)) {
    return false;
  }
  const char* begin = hex_string.data();
  const char* end = begin + hex_string.size();
  auto [ptr, ec] = std::from_chars(begin, end, result, 16);
  return ec == std::errc{} && ptr == end;
}

/**
 * Converts uint64_t to hex string with specified width.
 */
std::string toHexString(uint64_t value, int width) {
  std::ostringstream ss;
  ss << std::hex << std::setfill('0') << std::setw(width) << value;
  return ss.str();
}

} // namespace

// TraceId implementation

absl::StatusOr<TraceId> TraceId::fromHexString(absl::string_view hex_string) {
  if (hex_string.empty()) {
    return absl::InvalidArgumentError("Trace ID cannot be empty");
  }

  if (hex_string.size() == 16) {
    // 64-bit trace ID
    uint64_t value;
    if (!parseHexString(hex_string, value)) {
      return absl::InvalidArgumentError(
          absl::StrCat("Invalid 64-bit trace ID: ", hex_string));
    }
    if (value == 0) {
      return absl::InvalidArgumentError("Trace ID cannot be zero");
    }
    return TraceId(0, value);
  } else if (hex_string.size() == 32) {
    // 128-bit trace ID
    uint64_t high, low;
    if (!parseHexString(hex_string.substr(0, 16), high) ||
        !parseHexString(hex_string.substr(16, 16), low)) {
      return absl::InvalidArgumentError(
          absl::StrCat("Invalid 128-bit trace ID: ", hex_string));
    }
    if (high == 0 && low == 0) {
      return absl::InvalidArgumentError("Trace ID cannot be zero");
    }
    return TraceId(high, low);
  } else {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid trace ID length: ", hex_string.size(), 
                     " (must be 16 or 32 characters)"));
  }
}

TraceId TraceId::from128Bit(uint64_t high, uint64_t low) {
  return TraceId(high, low);
}

TraceId TraceId::from64Bit(uint64_t value) {
  return TraceId(0, value);
}

std::string TraceId::toHexString() const {
  if (is128Bit()) {
    return toHexString(high_, 16) + toHexString(low_, 16);
  } else {
    return toHexString(low_, 16);
  }
}

// SpanId implementation

absl::StatusOr<SpanId> SpanId::fromHexString(absl::string_view hex_string) {
  if (hex_string.size() != 16) {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid span ID length: ", hex_string.size(), 
                     " (must be 16 characters)"));
  }

  uint64_t value;
  if (!parseHexString(hex_string, value)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid span ID: ", hex_string));
  }

  if (value == 0) {
    return absl::InvalidArgumentError("Span ID cannot be zero");
  }

  return SpanId(value);
}

SpanId SpanId::from64Bit(uint64_t value) {
  return SpanId(value);
}

std::string SpanId::toHexString() const {
  return toHexString(value_, 16);
}

// TraceContext implementation

TraceContext::TraceContext(const TraceId& trace_id, const SpanId& span_id,
                          const absl::optional<SpanId>& parent_span_id,
                          SamplingState sampling_state, bool debug)
    : trace_id_(trace_id), span_id_(span_id), parent_span_id_(parent_span_id),
      sampling_state_(sampling_state), debug_(debug) {}

absl::StatusOr<std::string> TraceContext::toSingleHeader() const {
  if (!isValid()) {
    return absl::InvalidArgumentError("Invalid trace context: missing trace ID or span ID");
  }

  std::string result = trace_id_.toHexString() + "-" + span_id_.toHexString();

  // Add sampling state if specified
  if (sampling_state_ != SamplingState::UNSPECIFIED || debug_) {
    result += "-";
    switch (sampling_state_) {
      case SamplingState::NOT_SAMPLED:
        result += "0";
        break;
      case SamplingState::SAMPLED:
        result += "1";
        break;
      case SamplingState::DEBUG:
        result += "d";
        break;
      case SamplingState::UNSPECIFIED:
        if (debug_) {
          result += "d";
        } else {
          // Should not happen, but default to not sampled
          result += "0";
        }
        break;
    }
  }

  // Add parent span ID if present
  if (parent_span_id_.has_value()) {
    if (sampling_state_ == SamplingState::UNSPECIFIED && !debug_) {
      // Need to add sampling state if not already present
      result += "-0";
    }
    result += "-" + parent_span_id_.value().toHexString();
  }

  return result;
}

} // namespace B3
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
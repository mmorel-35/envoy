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

// Constants for improved maintainability
constexpr char kEmptyTraceIdMessage[] = "Trace ID cannot be empty";
constexpr char kZeroTraceIdMessage[] = "Trace ID cannot be zero";
constexpr char kZeroSpanIdMessage[] = "Span ID cannot be zero";
constexpr char kInvalidTraceIdLengthMessage[] = "Invalid trace ID length";
constexpr char kInvalidSpanIdLengthMessage[] = "Invalid span ID length";
constexpr char kInvalidContextMessage[] = "Invalid trace context: missing trace ID or span ID";

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

/**
 * Validates trace ID length and format.
 */
absl::Status validateTraceIdFormat(absl::string_view hex_string) {
  if (hex_string.empty()) {
    return absl::InvalidArgumentError(kEmptyTraceIdMessage);
  }

  if (hex_string.size() != 16 && hex_string.size() != 32) {
    return absl::InvalidArgumentError(
        absl::StrCat(kInvalidTraceIdLengthMessage, ": ", hex_string.size(), 
                     " (must be 16 or 32 characters)"));
  }

  return absl::OkStatus();
}

/**
 * Validates span ID length and format.
 */
absl::Status validateSpanIdFormat(absl::string_view hex_string) {
  if (hex_string.size() != 16) {
    return absl::InvalidArgumentError(
        absl::StrCat(kInvalidSpanIdLengthMessage, ": ", hex_string.size(), 
                     " (must be 16 characters)"));
  }

  return absl::OkStatus();
}

/**
 * Parses and validates 64-bit trace ID.
 */
absl::StatusOr<TraceId> parse64BitTraceId(absl::string_view hex_string) {
  uint64_t value;
  if (!parseHexString(hex_string, value)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid 64-bit trace ID: ", hex_string));
  }
  if (value == 0) {
    return absl::InvalidArgumentError(kZeroTraceIdMessage);
  }
  return TraceId::from64Bit(value);
}

/**
 * Parses and validates 128-bit trace ID.
 */
absl::StatusOr<TraceId> parse128BitTraceId(absl::string_view hex_string) {
  uint64_t high, low;
  if (!parseHexString(hex_string.substr(0, 16), high) ||
      !parseHexString(hex_string.substr(16, 16), low)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid 128-bit trace ID: ", hex_string));
  }
  if (high == 0 && low == 0) {
    return absl::InvalidArgumentError(kZeroTraceIdMessage);
  }
  return TraceId::from128Bit(high, low);
}

/**
 * Determines sampling state string representation.
 */
std::string getSamplingStateString(SamplingState sampling_state, bool debug) {
  if (debug) {
    return "d";
  }
  
  switch (sampling_state) {
    case SamplingState::NOT_SAMPLED:
      return "0";
    case SamplingState::SAMPLED:
      return "1";
    case SamplingState::DEBUG:
      return "d";
    case SamplingState::UNSPECIFIED:
      return "0"; // Default to not sampled
  }
  return "0";
}

/**
 * Builds single header string with optional components.
 */
std::string buildSingleHeaderWithOptionalFields(const TraceContext& context) {
  std::string result = context.traceId().toHexString() + "-" + context.spanId().toHexString();

  // Add sampling state if specified or debug flag is set
  if (context.samplingState() != SamplingState::UNSPECIFIED || context.debug()) {
    result += "-" + getSamplingStateString(context.samplingState(), context.debug());
  }

  // Add parent span ID if present
  if (context.parentSpanId().has_value()) {
    if (context.samplingState() == SamplingState::UNSPECIFIED && !context.debug()) {
      // Need to add sampling state if not already present
      result += "-0";
    }
    result += "-" + context.parentSpanId().value().toHexString();
  }

  return result;
}

} // namespace

// TraceId implementation

absl::StatusOr<TraceId> TraceId::fromHexString(absl::string_view hex_string) {
  auto format_validation = validateTraceIdFormat(hex_string);
  if (!format_validation.ok()) {
    return format_validation;
  }

  if (hex_string.size() == 16) {
    return parse64BitTraceId(hex_string);
  } else {
    return parse128BitTraceId(hex_string);
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
  auto format_validation = validateSpanIdFormat(hex_string);
  if (!format_validation.ok()) {
    return format_validation;
  }

  uint64_t value;
  if (!parseHexString(hex_string, value)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid span ID: ", hex_string));
  }

  if (value == 0) {
    return absl::InvalidArgumentError(kZeroSpanIdMessage);
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
    return absl::InvalidArgumentError(kInvalidContextMessage);
  }

  return buildSingleHeaderWithOptionalFields(*this);
}

} // namespace B3
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
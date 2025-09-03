#pragma once

#include <cstdint>
#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace B3 {

/**
 * Represents a B3 trace ID, which can be either 64-bit or 128-bit.
 * Provides validation and conversion functionality according to B3 specification.
 */
class TraceId {
public:
  /**
   * Creates a TraceId from a hex string representation.
   * @param hex_string The hex string (16 or 32 characters)
   * @return Valid TraceId or error status
   */
  static absl::StatusOr<TraceId> fromHexString(absl::string_view hex_string);

  /**
   * Creates a TraceId from high and low 64-bit values (for 128-bit trace IDs).
   */
  static TraceId from128Bit(uint64_t high, uint64_t low);

  /**
   * Creates a TraceId from a single 64-bit value.
   */
  static TraceId from64Bit(uint64_t value);

  /**
   * Default constructor creates an invalid trace ID.
   */
  TraceId() = default;

  /**
   * @return true if this trace ID is valid (non-zero)
   */
  bool isValid() const { return low_ != 0 || high_ != 0; }

  /**
   * @return true if this is a 128-bit trace ID
   */
  bool is128Bit() const { return high_ != 0; }

  /**
   * @return the high 64 bits (0 for 64-bit trace IDs)
   */
  uint64_t high() const { return high_; }

  /**
   * @return the low 64 bits
   */
  uint64_t low() const { return low_; }

  /**
   * @return hex string representation (16 or 32 characters)
   */
  std::string toHexString() const;

  bool operator==(const TraceId& other) const {
    return high_ == other.high_ && low_ == other.low_;
  }

private:
  TraceId(uint64_t high, uint64_t low) : high_(high), low_(low) {}

  uint64_t high_{0};
  uint64_t low_{0};
};

/**
 * Represents a B3 span ID (always 64-bit).
 */
class SpanId {
public:
  /**
   * Creates a SpanId from a hex string representation.
   * @param hex_string The hex string (16 characters)
   * @return Valid SpanId or error status
   */
  static absl::StatusOr<SpanId> fromHexString(absl::string_view hex_string);

  /**
   * Creates a SpanId from a 64-bit value.
   */
  static SpanId from64Bit(uint64_t value);

  /**
   * Default constructor creates an invalid span ID.
   */
  SpanId() = default;

  /**
   * @return true if this span ID is valid (non-zero)
   */
  bool isValid() const { return value_ != 0; }

  /**
   * @return the 64-bit value
   */
  uint64_t value() const { return value_; }

  /**
   * @return hex string representation (16 characters)
   */
  std::string toHexString() const;

  bool operator==(const SpanId& other) const { return value_ == other.value_; }

private:
  explicit SpanId(uint64_t value) : value_(value) {}

  uint64_t value_{0};
};

/**
 * Represents B3 sampling state.
 */
enum class SamplingState {
  NOT_SAMPLED,  // "0"
  SAMPLED,      // "1" 
  DEBUG,        // "d" (debug sampling)
  UNSPECIFIED   // Not present in headers
};

/**
 * Converts string to B3 sampling state per specification.
 * Supports: "0", "1", "d", "true", "false" (case insensitive)
 * @param value The sampling value string
 * @return SamplingState enum value
 */
SamplingState samplingStateFromString(absl::string_view value);

/**
 * Converts B3 sampling state to string representation.
 * @param state The sampling state
 * @return String representation ("0", "1", "d")
 */
std::string samplingStateToString(SamplingState state);

/**
 * Represents a complete B3 trace context with all propagation headers.
 */
class TraceContext {
public:
  /**
   * Default constructor creates an empty trace context.
   */
  TraceContext() = default;

  /**
   * Constructor with all B3 components.
   */
  TraceContext(const TraceId& trace_id, const SpanId& span_id, 
               const absl::optional<SpanId>& parent_span_id = absl::nullopt,
               SamplingState sampling_state = SamplingState::UNSPECIFIED,
               bool debug = false);

  /**
   * @return the trace ID
   */
  const TraceId& traceId() const { return trace_id_; }

  /**
   * @return the span ID
   */
  const SpanId& spanId() const { return span_id_; }

  /**
   * @return the parent span ID (if present)
   */
  const absl::optional<SpanId>& parentSpanId() const { return parent_span_id_; }

  /**
   * @return true if parent span ID is present
   */
  bool hasParentSpanId() const { return parent_span_id_.has_value(); }

  /**
   * @return the sampling state
   */
  SamplingState samplingState() const { return sampling_state_; }

  /**
   * @return true if debug flag is set
   */
  bool debug() const { return debug_; }

  /**
   * @return true if debug sampling is enabled
   */
  bool isDebug() const { return sampling_state_ == SamplingState::DEBUG; }

  /**
   * @return true if sampling decision is true (sampled or debug)
   */
  bool isSampled() const {
    return sampling_state_ == SamplingState::SAMPLED || 
           sampling_state_ == SamplingState::DEBUG;
  }

  /**
   * @return true if this trace context is valid (has valid trace and span IDs)
   */
  bool isValid() const { return trace_id_.isValid() && span_id_.isValid(); }

  /**
   * Sets the trace ID.
   */
  void setTraceId(const TraceId& trace_id) { trace_id_ = trace_id; }

  /**
   * Sets the span ID.
   */
  void setSpanId(const SpanId& span_id) { span_id_ = span_id; }

  /**
   * Sets the parent span ID.
   */
  void setParentSpanId(const absl::optional<SpanId>& parent_span_id) { 
    parent_span_id_ = parent_span_id; 
  }

  /**
   * Sets the sampling state.
   */
  void setSamplingState(SamplingState sampling_state) { 
    sampling_state_ = sampling_state; 
  }

  /**
   * Sets the debug flag.
   */
  void setDebug(bool debug) { debug_ = debug; }

  /**
   * Converts to B3 single header format.
   * @return B3 single header string or error status
   */
  absl::StatusOr<std::string> toSingleHeader() const;

  bool operator==(const TraceContext& other) const {
    return trace_id_ == other.trace_id_ && 
           span_id_ == other.span_id_ &&
           parent_span_id_ == other.parent_span_id_ &&
           sampling_state_ == other.sampling_state_ &&
           debug_ == other.debug_;
  }

private:
  TraceId trace_id_;
  SpanId span_id_;
  absl::optional<SpanId> parent_span_id_;
  SamplingState sampling_state_{SamplingState::UNSPECIFIED};
  bool debug_{false};
};

} // namespace B3
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
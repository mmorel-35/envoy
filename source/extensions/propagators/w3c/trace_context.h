#pragma once

#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3C {

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

/**
 * Represents a W3C traceparent header value.
 * Format: version-trace-id-parent-id-trace-flags
 * See https://www.w3.org/TR/trace-context/#traceparent-header
 */
class TraceParent {
public:
  /**
   * Construct a TraceParent from parsed components.
   * @param version the version field (2 hex characters)
   * @param trace_id the trace-id field (32 hex characters)
   * @param parent_id the parent-id field (16 hex characters)  
   * @param trace_flags the trace-flags field (2 hex characters)
   */
  TraceParent(absl::string_view version, absl::string_view trace_id,
              absl::string_view parent_id, absl::string_view trace_flags);

  /**
   * Parse a traceparent header value into a TraceParent object.
   * @param traceparent_value the raw traceparent header value
   * @return TraceParent object or error status if parsing fails
   */
  static absl::StatusOr<TraceParent> parse(absl::string_view traceparent_value);

  /**
   * Serialize this TraceParent to a traceparent header value.
   * @return the formatted traceparent header value
   */
  std::string toString() const;

  // Accessors following W3C terminology
  const std::string& version() const { return version_; }
  const std::string& traceId() const { return trace_id_; }
  const std::string& parentId() const { return parent_id_; }
  const std::string& traceFlags() const { return trace_flags_; }

  /**
   * Check if the sampled flag is set.
   * @return true if the trace is sampled
   */
  bool isSampled() const;

  /**
   * Set the sampled flag.
   * @param sampled whether the trace should be sampled
   */
  void setSampled(bool sampled);

private:
  std::string version_;
  std::string trace_id_;
  std::string parent_id_;
  std::string trace_flags_;

  // Validation helpers
  static bool isValidHex(absl::string_view input);
  static bool isAllZeros(absl::string_view input);
};

/**
 * Represents a W3C tracestate header value.
 * Contains vendor-specific trace information as key-value pairs.
 * See https://www.w3.org/TR/trace-context/#tracestate-header
 */
class TraceState {
public:
  /**
   * Construct an empty TraceState.
   */
  TraceState() = default;

  /**
   * Construct a TraceState from a tracestate header value.
   * @param tracestate_value the raw tracestate header value
   */
  explicit TraceState(absl::string_view tracestate_value);

  /**
   * Parse a tracestate header value into a TraceState object.
   * @param tracestate_value the raw tracestate header value
   * @return TraceState object or error status if parsing fails
   */
  static absl::StatusOr<TraceState> parse(absl::string_view tracestate_value);

  /**
   * Serialize this TraceState to a tracestate header value.
   * @return the formatted tracestate header value
   */
  std::string toString() const;

  /**
   * Get a value by key.
   * @param key the key to look up
   * @return the value if found, nullopt otherwise
   */
  absl::optional<absl::string_view> get(absl::string_view key) const;

  /**
   * Set a key-value pair.
   * @param key the key
   * @param value the value
   */
  void set(absl::string_view key, absl::string_view value);

  /**
   * Remove a key-value pair.
   * @param key the key to remove
   */
  void remove(absl::string_view key);

  /**
   * Check if the tracestate is empty.
   * @return true if no key-value pairs are present
   */
  bool empty() const { return entries_.empty(); }

private:
  // Store as vector to preserve order (required by W3C spec)
  std::vector<std::pair<std::string, std::string>> entries_;

  // Validation helpers
  static bool isValidKey(absl::string_view key);
  static bool isValidValue(absl::string_view value);
};

/**
 * Complete W3C trace context containing both traceparent and tracestate.
 * This represents the full context as defined by the W3C specification.
 */
class TraceContext {
public:
  /**
   * Construct with traceparent only.
   */
  explicit TraceContext(TraceParent traceparent);

  /**
   * Construct with both traceparent and tracestate.
   */
  TraceContext(TraceParent traceparent, TraceState tracestate);

  // Accessors
  const TraceParent& traceParent() const { return traceparent_; }
  TraceParent& traceParent() { return traceparent_; }
  
  const TraceState& traceState() const { return tracestate_; }
  TraceState& traceState() { return tracestate_; }

  /**
   * Check if tracestate is present.
   */
  bool hasTraceState() const { return !tracestate_.empty(); }

private:
  TraceParent traceparent_;
  TraceState tracestate_;
};

} // namespace W3C
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
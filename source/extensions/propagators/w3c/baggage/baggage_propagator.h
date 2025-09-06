#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "envoy/http/header_map.h"
#include "envoy/tracing/trace_context.h"

#include "source/common/singleton/const_singleton.h"
#include "source/common/tracing/trace_context_impl.h"

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3c {
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

/**
 * W3C Baggage constants for header names as defined in:
 * https://www.w3.org/TR/baggage/
 */
class BaggageConstantValues {
public:
  const Tracing::TraceContextHandler BAGGAGE{std::string(Constants::kBaggageHeader)};
};

using BaggageConstants = ConstSingleton<BaggageConstantValues>;

/**
 * Represents a single baggage member with key, value, and optional properties.
 */
struct BaggageMember {
  std::string key;
  std::string value;
  std::vector<std::string> properties;
};

/**
 * Map of baggage key to baggage member.
 */
using BaggageMap = std::unordered_map<std::string, BaggageMember>;

/**
 * W3C Baggage propagator implementation.
 * Provides extraction and injection of W3C baggage headers according to:
 * https://www.w3.org/TR/baggage/
 */
class BaggagePropagator {
public:
  BaggagePropagator();

  /**
   * Extract the baggage header value from trace context.
   * @param trace_context the trace context to extract from
   * @return the baggage header value if present
   */
  absl::optional<std::string> extractBaggage(const Tracing::TraceContext& trace_context) const;

  /**
   * Parse a baggage header value according to W3C specification.
   * @param baggage_value the baggage header value to parse
   * @return parsed baggage map or error status
   */
  absl::StatusOr<BaggageMap> parseBaggage(absl::string_view baggage_value) const;

  /**
   * Serialize a baggage map to header value format.
   * @param baggage_map the baggage map to serialize
   * @return serialized baggage header value
   */
  std::string serializeBaggage(const BaggageMap& baggage_map) const;

  /**
   * Inject baggage into trace context from a baggage map.
   * @param trace_context the trace context to inject into
   * @param baggage_map the baggage map to inject
   */
  void injectBaggage(Tracing::TraceContext& trace_context, const BaggageMap& baggage_map) const;

  /**
   * Inject baggage into trace context from a baggage string.
   * @param trace_context the trace context to inject into
   * @param baggage_value the baggage value to inject
   */
  void injectBaggage(Tracing::TraceContext& trace_context, absl::string_view baggage_value) const;

  /**
   * Remove baggage header from trace context.
   * @param trace_context the trace context to remove from
   */
  void removeBaggage(Tracing::TraceContext& trace_context) const;

  /**
   * Check if baggage header is present in trace context.
   * @param trace_context the trace context to check
   * @return true if baggage header is present
   */
  bool hasBaggage(const Tracing::TraceContext& trace_context) const;

  /**
   * Get a specific baggage value by key.
   * @param trace_context the trace context to search in
   * @param key the baggage key to retrieve
   * @return the baggage value or error status
   */
  absl::StatusOr<std::string> getBaggageValue(const Tracing::TraceContext& trace_context,
                                              absl::string_view key) const;

  /**
   * Set a specific baggage key-value pair.
   * @param trace_context the trace context to set in
   * @param key the baggage key to set
   * @param value the baggage value to set
   */
  void setBaggageValue(Tracing::TraceContext& trace_context,
                       absl::string_view key,
                       absl::string_view value) const;
};

} // namespace Baggage

// Convenient alias for easier usage, similar to other propagators like SkyWalking and XRay
using W3cBaggageConstants = Baggage::BaggageConstants;

} // namespace W3c
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

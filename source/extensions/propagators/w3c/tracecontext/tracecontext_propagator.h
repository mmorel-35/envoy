#pragma once

#include <cstdint>
#include <string>

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

/**
 * W3C Trace Context constants for header names as defined in:
 * https://www.w3.org/TR/trace-context/
 */
class TraceContextConstantValues {
public:
  const Tracing::TraceContextHandler TRACE_PARENT{std::string(Constants::kTraceparentHeader)};
  const Tracing::TraceContextHandler TRACE_STATE{std::string(Constants::kTracestateHeader)};
};

using TraceContextConstants = ConstSingleton<TraceContextConstantValues>;

/**
 * Parsed W3C traceparent header information.
 */
struct TraceParentInfo {
  std::string version;
  std::string trace_id;
  std::string span_id;
  bool sampled;
};

/**
 * W3C Trace Context propagator implementation.
 * Provides extraction and injection of W3C trace context headers according to:
 * https://www.w3.org/TR/trace-context/
 */
class TraceContextPropagator {
public:
  TraceContextPropagator();

  /**
   * Extract the traceparent header value from trace context.
   * @param trace_context the trace context to extract from
   * @return the traceparent header value if present
   */
  absl::optional<std::string> extractTraceParent(const Tracing::TraceContext& trace_context) const;

  /**
   * Extract the tracestate header value from trace context.
   * @param trace_context the trace context to extract from
   * @return the tracestate header value if present
   */
  absl::optional<std::string> extractTraceState(const Tracing::TraceContext& trace_context) const;

  /**
   * Parse a traceparent header value according to W3C specification.
   * @param traceparent_value the traceparent header value to parse
   * @return parsed trace parent information or error status
   */
  absl::StatusOr<TraceParentInfo> parseTraceParent(absl::string_view traceparent_value) const;

  /**
   * Inject traceparent header into trace context.
   * @param trace_context the trace context to inject into
   * @param version the W3C version (typically "00")
   * @param trace_id the trace ID (32 hex characters)
   * @param span_id the span ID (16 hex characters)
   * @param sampled whether the trace is sampled
   */
  void injectTraceParent(Tracing::TraceContext& trace_context, absl::string_view version,
                         absl::string_view trace_id, absl::string_view span_id, bool sampled) const;

  /**
   * Inject tracestate header into trace context.
   * @param trace_context the trace context to inject into
   * @param tracestate the tracestate value to inject
   */
  void injectTraceState(Tracing::TraceContext& trace_context, absl::string_view tracestate) const;

  /**
   * Remove traceparent header from trace context.
   * @param trace_context the trace context to remove from
   */
  void removeTraceParent(Tracing::TraceContext& trace_context) const;

  /**
   * Remove tracestate header from trace context.
   * @param trace_context the trace context to remove from
   */
  void removeTraceState(Tracing::TraceContext& trace_context) const;

  /**
   * Check if traceparent header is present in trace context.
   * @param trace_context the trace context to check
   * @return true if traceparent header is present
   */
  bool hasTraceParent(const Tracing::TraceContext& trace_context) const;
};

} // namespace TraceContext

// Convenient alias for easier usage, similar to other propagators like SkyWalking and XRay
using W3cConstants = TraceContext::TraceContextConstants;

} // namespace W3c
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

#pragma once

#include "source/extensions/propagators/w3c/trace_context.h"

#include "envoy/tracing/trace_context.h"
#include "source/common/tracing/trace_context_impl.h"

#include "absl/status/statusor.h"
#include "absl/types/optional.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3C {

/**
 * W3C Trace Context constants for header handling.
 */
class W3CConstantValues {
public:
  const Tracing::TraceContextHandler TRACE_PARENT{std::string(Constants::kTraceparentHeader)};
  const Tracing::TraceContextHandler TRACE_STATE{std::string(Constants::kTracestateHeader)};
};

using W3CConstants = ConstSingleton<W3CConstantValues>;

/**
 * W3C Trace Context Propagator.
 * Provides extraction and injection of W3C trace context headers.
 * 
 * This component implements the W3C Trace Context specification:
 * https://www.w3.org/TR/trace-context/
 * 
 * It handles both traceparent and tracestate headers and provides
 * a reusable interface for different tracing systems.
 */
class Propagator {
public:
  /**
   * Check if W3C trace context headers are present.
   * @param trace_context the trace context to check
   * @return true if traceparent header is present
   */
  static bool isPresent(const Tracing::TraceContext& trace_context);

  /**
   * Extract W3C trace context from headers.
   * @param trace_context the trace context containing headers
   * @return W3C TraceContext or error status if extraction fails
   */
  static absl::StatusOr<TraceContext> extract(const Tracing::TraceContext& trace_context);

  /**
   * Inject W3C trace context into headers.
   * @param w3c_context the W3C trace context to inject
   * @param trace_context the trace context to inject headers into
   */
  static void inject(const TraceContext& w3c_context, Tracing::TraceContext& trace_context);

  /**
   * Create a new trace context with a new span.
   * @param parent_context the parent W3C trace context
   * @param new_span_id the new span ID (32 hex characters)
   * @return new W3C TraceContext with updated parent-id
   */
  static absl::StatusOr<TraceContext> createChild(const TraceContext& parent_context,
                                                  absl::string_view new_span_id);

  /**
   * Create a new root trace context.
   * @param trace_id the trace ID (32 hex characters)
   * @param span_id the span ID (16 hex characters)
   * @param sampled whether the trace is sampled
   * @return new W3C TraceContext
   */
  static absl::StatusOr<TraceContext> createRoot(absl::string_view trace_id,
                                                 absl::string_view span_id,
                                                 bool sampled);

private:
  // Helper to validate hex string of specific length
  static bool isValidHexString(absl::string_view input, size_t expected_length);
};

/**
 * Utility class for working with W3C trace context in existing Envoy tracers.
 * Provides backward compatibility and easy integration.
 */
class TracingHelper {
public:
  /**
   * Extract trace parent information in a format compatible with existing tracers.
   * @param trace_context the trace context containing headers
   * @return struct containing extracted values or nullopt if not present/invalid
   */
  struct ExtractedContext {
    std::string version;
    std::string trace_id;
    std::string span_id;
    std::string trace_flags;
    bool sampled;
    std::string tracestate;
  };

  static absl::optional<ExtractedContext> extractForTracer(const Tracing::TraceContext& trace_context);

  /**
   * Check if traceparent header is present (for backward compatibility).
   */
  static bool traceparentPresent(const Tracing::TraceContext& trace_context);
};

} // namespace W3C
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
#pragma once

#include "envoy/tracing/tracer.h"

#include "source/extensions/propagators/b3/trace_context.h"

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace B3 {

/**
 * B3 Propagator provides comprehensive support for extracting and injecting
 * B3 distributed tracing headers according to the B3 specification.
 * 
 * Supports both multiple headers and single header formats:
 * - Multiple headers: x-b3-traceid, x-b3-spanid, x-b3-parentspanid, x-b3-sampled, x-b3-flags
 * - Single header: b3 (format: {traceid}-{spanid}-{sampled}-{parentspanid})
 * 
 * Features:
 * - 64-bit and 128-bit trace ID support
 * - Proper sampling state management including debug sampling
 * - Comprehensive validation and error handling
 * - Clean separation between data structures and business logic
 */
class Propagator {
public:
  /**
   * Checks if B3 headers are present in the trace context.
   * @param trace_context The trace context to check
   * @return true if any B3 headers are found
   */
  static bool isPresent(const Tracing::TraceContext& trace_context);

  /**
   * Extracts B3 trace context from HTTP headers.
   * Supports both multiple headers and single header formats.
   * @param trace_context The trace context containing headers
   * @return B3 TraceContext or error status if extraction fails
   */
  static absl::StatusOr<TraceContext> extract(const Tracing::TraceContext& trace_context);

  /**
   * Injects B3 trace context into HTTP headers.
   * Prefers multiple headers format for maximum compatibility.
   * @param b3_context The B3 trace context to inject
   * @param trace_context The target trace context for header injection
   * @return Success status or error if injection fails
   */
  static absl::Status inject(const TraceContext& b3_context,
                            Tracing::TraceContext& trace_context);

  /**
   * Extracts B3 trace context using single header format only.
   * @param trace_context The trace context containing headers
   * @return B3 TraceContext or error status
   */
  static absl::StatusOr<TraceContext> extractSingleHeader(
      const Tracing::TraceContext& trace_context);

  /**
   * Injects B3 trace context using single header format only.
   * @param b3_context The B3 trace context to inject
   * @param trace_context The target trace context for header injection
   * @return Success status or error if injection fails
   */
  static absl::Status injectSingleHeader(const TraceContext& b3_context,
                                        Tracing::TraceContext& trace_context);

  /**
   * Extracts B3 trace context using multiple headers format only.
   * @param trace_context The trace context containing headers
   * @return B3 TraceContext or error status
   */
  static absl::StatusOr<TraceContext> extractMultipleHeaders(
      const Tracing::TraceContext& trace_context);

  /**
   * Injects B3 trace context using multiple headers format only.
   * @param b3_context The B3 trace context to inject  
   * @param trace_context The target trace context for header injection
   * @return Success status or error if injection fails
   */
  static absl::Status injectMultipleHeaders(const TraceContext& b3_context,
                                           Tracing::TraceContext& trace_context);

private:
  /**
   * Parses sampling state from string value.
   * @param value The sampling value ("0", "1", "d", "true", etc.)
   * @return SamplingState enum value
   */
  static SamplingState parseSamplingState(absl::string_view value);

  /**
   * Converts SamplingState to string representation.
   * @param state The sampling state
   * @return String representation ("0", "1", "d")
   */
  static std::string samplingStateToString(SamplingState state);
};

/**
 * TracingHelper provides backward compatibility interface for existing tracers
 * to seamlessly integrate with B3 propagation.
 */
class TracingHelper {
public:
  /**
   * Extracts B3 trace context specifically for tracer consumption.
   * Provides a simplified interface that matches existing tracer expectations.
   * @param trace_context The trace context containing headers
   * @return B3 TraceContext optimized for tracer use, or nullopt if not present
   */
  static absl::optional<TraceContext> extractForTracer(
      const Tracing::TraceContext& trace_context);

  /**
   * Injects B3 trace context from tracer-generated data.
   * @param b3_context The B3 context to inject
   * @param trace_context The target trace context
   * @return Success status
   */
  static absl::Status injectFromTracer(const TraceContext& b3_context,
                                      Tracing::TraceContext& trace_context);

  /**
   * Converts B3 sampling state to boolean for tracer compatibility.
   * @param sampling_state The B3 sampling state
   * @return true if sampled (including debug), false otherwise
   */
  static bool isSampled(SamplingState sampling_state);

  /**
   * Creates a B3 TraceContext from tracer-specific values.
   * @param trace_id_high High 64 bits of trace ID (0 for 64-bit)
   * @param trace_id_low Low 64 bits of trace ID 
   * @param span_id The span ID
   * @param parent_span_id The parent span ID (0 if none)
   * @param sampled Whether the trace is sampled
   * @return B3 TraceContext
   */
  static TraceContext createTraceContext(uint64_t trace_id_high, uint64_t trace_id_low,
                                        uint64_t span_id, uint64_t parent_span_id,
                                        bool sampled);
};

} // namespace B3
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
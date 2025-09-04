#pragma once

#include "source/extensions/propagators/propagator_interface.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3C {

/**
 * W3C Trace Context propagator implementation.
 * 
 * Implements the W3C Trace Context specification:
 * https://www.w3.org/TR/trace-context/
 * 
 * Headers:
 * - traceparent: version-format trace ID, span ID, and trace flags
 *   Format: 00-{trace-id}-{parent-id}-{trace-flags}
 *   Example: 00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01
 * 
 * - tracestate: vendor-specific trace state (optional)
 *   Format: key1=value1,key2=value2
 *   Example: rojo=00f067aa0ba902b7,congo=t61rcWkgMzE
 */
class W3CPropagator : public Propagator {
public:
  W3CPropagator() = default;

  // Propagator interface
  TraceHeader extract(const Tracing::TraceContext& trace_context) const override;
  void inject(Tracing::TraceContext& trace_context, const TraceHeader& trace_header) const override;
  absl::string_view name() const override { return "w3c"; }

private:
  /**
   * Parse traceparent header.
   * Format: 00-{trace-id}-{parent-id}-{trace-flags}
   */
  TraceHeader parseTraceParent(absl::string_view traceparent) const;

  /**
   * Generate traceparent header.
   * Format: 00-{trace-id}-{parent-id}-{trace-flags}  
   */
  std::string formatTraceParent(const TraceHeader& trace_header) const;

  /**
   * Validate W3C trace ID format (32 hex characters).
   */
  bool isValidTraceId(absl::string_view trace_id) const;

  /**
   * Validate W3C span ID format (16 hex characters).
   */
  bool isValidSpanId(absl::string_view span_id) const;

  /**
   * Validate W3C trace flags format (2 hex characters).
   */
  bool isValidTraceFlags(absl::string_view trace_flags) const;

  /**
   * Convert trace flags to sampling decision.
   */
  absl::optional<bool> traceFlagsToSampled(absl::string_view trace_flags) const;

  /**
   * Convert sampling decision to trace flags.
   */
  std::string sampledToTraceFlags(absl::optional<bool> sampled) const;
};

} // namespace W3C
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
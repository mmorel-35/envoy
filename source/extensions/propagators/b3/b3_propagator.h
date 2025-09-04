#pragma once

#include "source/extensions/propagators/propagator_interface.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace B3 {

/**
 * B3 trace propagator implementation.
 * Supports both multi-header and single header B3 formats.
 * 
 * Multi-header format:
 * - X-B3-TraceId: 128-bit or 64-bit trace ID
 * - X-B3-SpanId: 64-bit span ID  
 * - X-B3-ParentSpanId: 64-bit parent span ID (optional)
 * - X-B3-Sampled: sampling decision ("1" or "0")
 * - X-B3-Flags: flags (currently only "1" for debug)
 * 
 * Single header format:
 * - b3: {TraceId}-{SpanId}-{SamplingState}-{ParentSpanId}
 * 
 * Reference: https://github.com/openzipkin/b3-propagation
 */
class B3Propagator : public Propagator {
public:
  B3Propagator() = default;

  // Propagator interface
  TraceHeader extract(const Tracing::TraceContext& trace_context) const override;
  void inject(Tracing::TraceContext& trace_context, const TraceHeader& trace_header) const override;
  absl::string_view name() const override { return "b3"; }

private:
  /**
   * Extract trace context from B3 multi-header format.
   */
  TraceHeader extractMultiHeader(const Tracing::TraceContext& trace_context) const;

  /**
   * Extract trace context from B3 single header format.
   */
  TraceHeader extractSingleHeader(const Tracing::TraceContext& trace_context) const;

  /**
   * Inject trace context using B3 multi-header format.
   */
  void injectMultiHeader(Tracing::TraceContext& trace_context, const TraceHeader& trace_header) const;

  /**
   * Inject trace context using B3 single header format.
   */
  void injectSingleHeader(Tracing::TraceContext& trace_context, const TraceHeader& trace_header) const;

  /**
   * Validate B3 trace ID format.
   */
  bool isValidTraceId(absl::string_view trace_id) const;

  /**
   * Validate B3 span ID format.
   */
  bool isValidSpanId(absl::string_view span_id) const;
};

} // namespace B3
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
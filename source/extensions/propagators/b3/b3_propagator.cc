#include "source/extensions/propagators/b3/b3_propagator.h"

#include "source/extensions/propagators/propagator_constants.h"
#include "source/common/common/hex.h"
#include "source/common/common/utility.h"

#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace B3 {

TraceHeader B3Propagator::extract(const Tracing::TraceContext& trace_context) const {
  // Try multi-header format first
  auto multi_header_result = extractMultiHeader(trace_context);
  if (multi_header_result.trace_id.has_value()) {
    return multi_header_result;
  }

  // Fall back to single header format
  return extractSingleHeader(trace_context);
}

void B3Propagator::inject(Tracing::TraceContext& trace_context, const TraceHeader& trace_header) const {
  // For now, always use multi-header format for injection
  // TODO: Add configuration to choose between single and multi-header format
  injectMultiHeader(trace_context, trace_header);
}

TraceHeader B3Propagator::extractMultiHeader(const Tracing::TraceContext& trace_context) const {
  const auto& constants = PropagatorConstants::get();
  TraceHeader header;

  // Extract trace ID
  if (auto trace_id = constants.X_B3_TRACE_ID.get(trace_context)) {
    if (isValidTraceId(*trace_id)) {
      header.trace_id = std::string(*trace_id);
    }
  }

  // Extract span ID
  if (auto span_id = constants.X_B3_SPAN_ID.get(trace_context)) {
    if (isValidSpanId(*span_id)) {
      header.span_id = std::string(*span_id);
    }
  }

  // Extract parent span ID (optional)
  if (auto parent_span_id = constants.X_B3_PARENT_SPAN_ID.get(trace_context)) {
    if (isValidSpanId(*parent_span_id)) {
      header.parent_span_id = std::string(*parent_span_id);
    }
  }

  // Extract sampling decision
  if (auto sampled = constants.X_B3_SAMPLED.get(trace_context)) {
    if (*sampled == "1") {
      header.sampled = true;
    } else if (*sampled == "0") {
      header.sampled = false;
    }
  }

  return header;
}

TraceHeader B3Propagator::extractSingleHeader(const Tracing::TraceContext& trace_context) const {
  const auto& constants = PropagatorConstants::get();
  TraceHeader header;

  auto b3_header = constants.B3.get(trace_context);
  if (!b3_header.has_value()) {
    return header;
  }

  // Parse B3 single header format: {TraceId}-{SpanId}-{SamplingState}-{ParentSpanId}
  std::vector<absl::string_view> parts = absl::StrSplit(*b3_header, '-');
  
  if (parts.size() >= 2) {
    // Trace ID (required)
    if (isValidTraceId(parts[0])) {
      header.trace_id = std::string(parts[0]);
    }

    // Span ID (required)
    if (isValidSpanId(parts[1])) {
      header.span_id = std::string(parts[1]);
    }

    // Sampling state (optional)
    if (parts.size() >= 3 && !parts[2].empty()) {
      if (parts[2] == "1" || parts[2] == "d") {
        header.sampled = true;
      } else if (parts[2] == "0") {
        header.sampled = false;
      }
    }

    // Parent span ID (optional)
    if (parts.size() >= 4 && !parts[3].empty()) {
      if (isValidSpanId(parts[3])) {
        header.parent_span_id = std::string(parts[3]);
      }
    }
  }

  return header;
}

void B3Propagator::injectMultiHeader(Tracing::TraceContext& trace_context, const TraceHeader& trace_header) const {
  const auto& constants = PropagatorConstants::get();

  // Inject trace ID
  if (trace_header.trace_id.has_value()) {
    constants.X_B3_TRACE_ID.set(trace_context, *trace_header.trace_id);
  }

  // Inject span ID
  if (trace_header.span_id.has_value()) {
    constants.X_B3_SPAN_ID.set(trace_context, *trace_header.span_id);
  }

  // Inject parent span ID
  if (trace_header.parent_span_id.has_value()) {
    constants.X_B3_PARENT_SPAN_ID.set(trace_context, *trace_header.parent_span_id);
  }

  // Inject sampling decision
  if (trace_header.sampled.has_value()) {
    constants.X_B3_SAMPLED.set(trace_context, *trace_header.sampled ? "1" : "0");
  }
}

void B3Propagator::injectSingleHeader(Tracing::TraceContext& trace_context, const TraceHeader& trace_header) const {
  // TODO: Implement single header injection
  // Format: {TraceId}-{SpanId}-{SamplingState}-{ParentSpanId}
}

bool B3Propagator::isValidTraceId(absl::string_view trace_id) const {
  // B3 trace IDs can be 64-bit (16 hex chars) or 128-bit (32 hex chars)
  return (trace_id.length() == 16 || trace_id.length() == 32) && 
         Hex::isValidHexString(trace_id);
}

bool B3Propagator::isValidSpanId(absl::string_view span_id) const {
  // B3 span IDs are 64-bit (16 hex chars)
  return span_id.length() == 16 && Hex::isValidHexString(span_id);
}

} // namespace B3
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
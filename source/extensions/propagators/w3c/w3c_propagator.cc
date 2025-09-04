#include "source/extensions/propagators/w3c/w3c_propagator.h"

#include "source/extensions/propagators/propagator_constants.h"
#include "source/common/common/hex.h"
#include "source/common/common/utility.h"

#include "absl/strings/str_split.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3C {

TraceHeader W3CPropagator::extract(const Tracing::TraceContext& trace_context) const {
  const auto& constants = PropagatorConstants::get();
  TraceHeader header;

  // Extract traceparent header
  if (auto traceparent = constants.TRACE_PARENT.get(trace_context)) {
    header = parseTraceParent(*traceparent);
  }

  // Extract tracestate header (store as trace_state field)
  if (auto tracestate = constants.TRACE_STATE.get(trace_context)) {
    header.trace_state = std::string(*tracestate);
  }

  return header;
}

void W3CPropagator::inject(Tracing::TraceContext& trace_context, const TraceHeader& trace_header) const {
  const auto& constants = PropagatorConstants::get();

  // Inject traceparent header
  if (trace_header.trace_id.has_value() && trace_header.span_id.has_value()) {
    auto traceparent = formatTraceParent(trace_header);
    constants.TRACE_PARENT.set(trace_context, traceparent);
  }

  // Inject tracestate header
  if (trace_header.trace_state.has_value()) {
    constants.TRACE_STATE.set(trace_context, *trace_header.trace_state);
  }
}

TraceHeader W3CPropagator::parseTraceParent(absl::string_view traceparent) const {
  TraceHeader header;

  // W3C traceparent format: 00-{trace-id}-{parent-id}-{trace-flags}
  std::vector<absl::string_view> parts = absl::StrSplit(traceparent, '-');
  
  if (parts.size() != 4) {
    return header; // Invalid format
  }

  // Check version (currently only version 00 is supported)
  if (parts[0] != "00") {
    return header; // Unsupported version
  }

  // Extract trace ID (32 hex characters)
  if (isValidTraceId(parts[1])) {
    header.trace_id = std::string(parts[1]);
  }

  // Extract span ID (16 hex characters) 
  if (isValidSpanId(parts[2])) {
    header.span_id = std::string(parts[2]);
  }

  // Extract trace flags and convert to sampling decision
  if (isValidTraceFlags(parts[3])) {
    header.sampled = traceFlagsToSampled(parts[3]);
  }

  return header;
}

std::string W3CPropagator::formatTraceParent(const TraceHeader& trace_header) const {
  // Default version: 00
  std::string version = "00";
  
  // Use provided trace_id or generate placeholder
  std::string trace_id = trace_header.trace_id.value_or("00000000000000000000000000000000");
  
  // Use provided span_id or generate placeholder  
  std::string span_id = trace_header.span_id.value_or("0000000000000000");
  
  // Convert sampling decision to trace flags
  std::string trace_flags = sampledToTraceFlags(trace_header.sampled);
  
  return absl::StrCat(version, "-", trace_id, "-", span_id, "-", trace_flags);
}

bool W3CPropagator::isValidTraceId(absl::string_view trace_id) const {
  // W3C trace ID: 32 hex characters (128-bit)
  return trace_id.length() == 32 && 
         Hex::isValidHexString(trace_id) &&
         trace_id != "00000000000000000000000000000000"; // Must not be all zeros
}

bool W3CPropagator::isValidSpanId(absl::string_view span_id) const {
  // W3C span ID: 16 hex characters (64-bit)
  return span_id.length() == 16 && 
         Hex::isValidHexString(span_id) &&
         span_id != "0000000000000000"; // Must not be all zeros
}

bool W3CPropagator::isValidTraceFlags(absl::string_view trace_flags) const {
  // W3C trace flags: 2 hex characters (8-bit)
  return trace_flags.length() == 2 && Hex::isValidHexString(trace_flags);
}

absl::optional<bool> W3CPropagator::traceFlagsToSampled(absl::string_view trace_flags) const {
  if (trace_flags.length() != 2) {
    return absl::nullopt;
  }

  // Parse the least significant bit (sampled flag)
  // The sampled flag is the least significant bit of trace-flags
  auto flags_value = Hex::decode(trace_flags);
  if (flags_value.empty()) {
    return absl::nullopt;
  }

  return (flags_value[0] & 0x01) != 0;
}

std::string W3CPropagator::sampledToTraceFlags(absl::optional<bool> sampled) const {
  // Default to not sampled (00)
  uint8_t flags = 0x00;
  
  // Set the sampled flag (least significant bit) if sampled is true
  if (sampled.has_value() && *sampled) {
    flags |= 0x01;
  }
  
  return Hex::encode(&flags, 1);
}

} // namespace W3C
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
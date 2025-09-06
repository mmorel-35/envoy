#include "source/extensions/propagators/w3c/tracecontext/tracecontext_propagator.h"

#include "source/common/common/hex.h"

#include "absl/strings/escaping.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3c {
namespace TraceContext {

namespace {

// W3C TraceContext validation helpers
bool isValidHex(absl::string_view input) {
  return std::all_of(input.begin(), input.end(),
                     [](const char& c) { return absl::ascii_isxdigit(c); });
}

bool isAllZeros(absl::string_view input) {
  return std::all_of(input.begin(), input.end(), [](const char& c) { return c == '0'; });
}

// Validate traceparent component sizes according to W3C specification
bool hasValidComponentSizes(absl::string_view version, absl::string_view trace_id,
                           absl::string_view span_id, absl::string_view trace_flags) {
  return version.size() == Constants::kVersionSize && 
         trace_id.size() == Constants::kTraceIdSize &&
         span_id.size() == Constants::kParentIdSize && 
         trace_flags.size() == Constants::kTraceFlagsSize;
}

// Validate that all components are valid hexadecimal
bool hasValidHexComponents(absl::string_view version, absl::string_view trace_id,
                          absl::string_view span_id, absl::string_view trace_flags) {
  return isValidHex(version) && isValidHex(trace_id) && 
         isValidHex(span_id) && isValidHex(trace_flags);
}

} // namespace

TraceContextPropagator::TraceContextPropagator() = default;

absl::optional<std::string> TraceContextPropagator::extractTraceParent(
    const Tracing::TraceContext& trace_context) const {
  auto header_value = TraceContextConstants::get().TRACE_PARENT.get(trace_context);
  return header_value;
}

absl::optional<std::string> TraceContextPropagator::extractTraceState(
    const Tracing::TraceContext& trace_context) const {
  auto tracestate_values = TraceContextConstants::get().TRACE_STATE.getAll(trace_context);
  if (tracestate_values.empty()) {
    return absl::nullopt;
  }
  return absl::StrJoin(tracestate_values, ",");
}

absl::StatusOr<TraceParentInfo> TraceContextPropagator::parseTraceParent(
    absl::string_view traceparent_value) const {
  
  if (traceparent_value.size() != Constants::kTraceparentHeaderSize) {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid traceparent header length: expected ", 
                     Constants::kTraceparentHeaderSize, ", got ", traceparent_value.size()));
  }

  // Split into components: version-trace-id-parent-id-trace-flags
  std::vector<absl::string_view> components =
      absl::StrSplit(traceparent_value, '-', absl::SkipEmpty());
  
  if (components.size() != 4) {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid traceparent format: expected 4 hyphen-separated components, got ", 
                     components.size()));
  }

  absl::string_view version = components[0];
  absl::string_view trace_id = components[1];
  absl::string_view span_id = components[2];
  absl::string_view trace_flags = components[3];

  // Validate field sizes
  if (!hasValidComponentSizes(version, trace_id, span_id, trace_flags)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid traceparent field sizes: version=", version.size(),
                     ", trace_id=", trace_id.size(), ", span_id=", span_id.size(),
                     ", trace_flags=", trace_flags.size()));
  }

  // Validate hex encoding
  if (!hasValidHexComponents(version, trace_id, span_id, trace_flags)) {
    return absl::InvalidArgumentError("Invalid traceparent hex encoding");
  }

  // Validate IDs are not all zeros (per W3C spec)
  if (isAllZeros(trace_id)) {
    return absl::InvalidArgumentError("Invalid trace_id: cannot be all zeros per W3C specification");
  }
  if (isAllZeros(span_id)) {
    return absl::InvalidArgumentError("Invalid span_id: cannot be all zeros per W3C specification");
  }

  // Parse trace flags for sampled bit
  char decoded_trace_flags = absl::HexStringToBytes(trace_flags).front();
  bool sampled = (decoded_trace_flags & Constants::kSampledFlag) != 0;

  return TraceParentInfo{
    std::string(version),
    std::string(trace_id), 
    std::string(span_id),
    sampled
  };
}

void TraceContextPropagator::injectTraceParent(
    Tracing::TraceContext& trace_context,
    absl::string_view version,
    absl::string_view trace_id,
    absl::string_view span_id,
    bool sampled) const {
  
  // Input validation
  if (version.size() != Constants::kVersionSize || !isValidHex(version)) {
    return; // Silently ignore invalid input
  }
  if (trace_id.size() != Constants::kTraceIdSize || !isValidHex(trace_id) || isAllZeros(trace_id)) {
    return; // Silently ignore invalid input
  }
  if (span_id.size() != Constants::kParentIdSize || !isValidHex(span_id) || isAllZeros(span_id)) {
    return; // Silently ignore invalid input
  }

  std::vector<uint8_t> trace_flags_vec{sampled ? Constants::kSampledFlag : uint8_t(0)};
  std::string trace_flags_hex = Hex::encode(trace_flags_vec);
  
  std::string traceparent_value = absl::StrCat(
    version, "-", trace_id, "-", span_id, "-", trace_flags_hex);
  
  TraceContextConstants::get().TRACE_PARENT.setRefKey(trace_context, traceparent_value);
}

void TraceContextPropagator::injectTraceState(
    Tracing::TraceContext& trace_context,
    absl::string_view tracestate) const {
  
  if (!tracestate.empty()) {
    TraceContextConstants::get().TRACE_STATE.setRefKey(trace_context, tracestate);
  }
}

void TraceContextPropagator::removeTraceParent(Tracing::TraceContext& trace_context) const {
  TraceContextConstants::get().TRACE_PARENT.remove(trace_context);
}

void TraceContextPropagator::removeTraceState(Tracing::TraceContext& trace_context) const {
  TraceContextConstants::get().TRACE_STATE.remove(trace_context);
}

bool TraceContextPropagator::hasTraceParent(const Tracing::TraceContext& trace_context) const {
  return TraceContextConstants::get().TRACE_PARENT.get(trace_context).has_value();
}

} // namespace TraceContext
} // namespace W3c
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
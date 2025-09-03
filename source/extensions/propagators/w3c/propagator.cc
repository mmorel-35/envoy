#include "source/extensions/propagators/w3c/propagator.h"

#include "absl/strings/str_join.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3C {

bool Propagator::isPresent(const Tracing::TraceContext& trace_context) {
  auto traceparent_header = W3CConstants::get().TRACE_PARENT.get(trace_context);
  return traceparent_header.has_value();
}

absl::StatusOr<TraceContext> Propagator::extract(const Tracing::TraceContext& trace_context) {
  // Extract traceparent header - required
  auto traceparent_header = W3CConstants::get().TRACE_PARENT.get(trace_context);
  if (!traceparent_header.has_value()) {
    return absl::InvalidArgumentError("No traceparent header found");
  }

  // Parse traceparent
  auto traceparent_result = TraceParent::parse(traceparent_header.value());
  if (!traceparent_result.ok()) {
    return traceparent_result.status();
  }

  // Extract tracestate header - optional
  TraceState tracestate;
  auto tracestate_headers = W3CConstants::get().TRACE_STATE.getAll(trace_context);
  if (!tracestate_headers.empty()) {
    // Join multiple tracestate headers with comma as per W3C spec
    std::string combined_tracestate = absl::StrJoin(tracestate_headers, ",");
    auto tracestate_result = TraceState::parse(combined_tracestate);
    if (tracestate_result.ok()) {
      tracestate = std::move(tracestate_result.value());
    }
    // Note: We don't fail on tracestate parsing errors as it's optional
  }

  return TraceContext(std::move(traceparent_result.value()), std::move(tracestate));
}

void Propagator::inject(const TraceContext& w3c_context, Tracing::TraceContext& trace_context) {
  // Inject traceparent header
  std::string traceparent_value = w3c_context.traceParent().toString();
  W3CConstants::get().TRACE_PARENT.set(trace_context, traceparent_value);

  // Inject tracestate header if present
  if (w3c_context.hasTraceState()) {
    std::string tracestate_value = w3c_context.traceState().toString();
    if (!tracestate_value.empty()) {
      W3CConstants::get().TRACE_STATE.set(trace_context, tracestate_value);
    }
  }
}

absl::StatusOr<TraceContext> Propagator::createChild(const TraceContext& parent_context,
                                                     absl::string_view new_span_id) {
  // Validate new span ID
  if (!isValidHexString(new_span_id, Constants::kParentIdSize)) {
    return absl::InvalidArgumentError("Invalid span ID: must be 16 hex characters");
  }

  // Create new traceparent with updated parent-id
  TraceParent child_traceparent(
      parent_context.traceParent().version(),
      parent_context.traceParent().traceId(),
      std::string(new_span_id),
      parent_context.traceParent().traceFlags()
  );

  // Copy tracestate from parent
  return TraceContext(std::move(child_traceparent), parent_context.traceState());
}

absl::StatusOr<TraceContext> Propagator::createRoot(absl::string_view trace_id,
                                                    absl::string_view span_id,
                                                    bool sampled) {
  // Validate inputs
  if (!isValidHexString(trace_id, Constants::kTraceIdSize)) {
    return absl::InvalidArgumentError("Invalid trace ID: must be 32 hex characters");
  }
  if (!isValidHexString(span_id, Constants::kParentIdSize)) {
    return absl::InvalidArgumentError("Invalid span ID: must be 16 hex characters");
  }

  // Create trace flags with sampled bit
  std::string trace_flags = sampled ? "01" : "00";

  // Create new traceparent
  TraceParent root_traceparent(
      std::string(Constants::kCurrentVersion),
      std::string(trace_id),
      std::string(span_id),
      trace_flags
  );

  return TraceContext(std::move(root_traceparent));
}

bool Propagator::isValidHexString(absl::string_view input, size_t expected_length) {
  if (input.size() != expected_length) {
    return false;
  }
  
  return std::all_of(input.begin(), input.end(),
                     [](char c) { return std::isxdigit(c); });
}

// TracingHelper implementation

absl::optional<TracingHelper::ExtractedContext> 
TracingHelper::extractForTracer(const Tracing::TraceContext& trace_context) {
  auto w3c_result = Propagator::extract(trace_context);
  if (!w3c_result.ok()) {
    return absl::nullopt;
  }

  const auto& w3c_context = w3c_result.value();
  const auto& traceparent = w3c_context.traceParent();

  ExtractedContext result;
  result.version = traceparent.version();
  result.trace_id = traceparent.traceId();
  result.span_id = traceparent.parentId();
  result.trace_flags = traceparent.traceFlags();
  result.sampled = traceparent.isSampled();
  result.tracestate = w3c_context.traceState().toString();

  return result;
}

bool TracingHelper::traceparentPresent(const Tracing::TraceContext& trace_context) {
  return Propagator::isPresent(trace_context);
}

} // namespace W3C
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
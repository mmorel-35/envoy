#include "source/extensions/propagators/type_converter.h"

#include "absl/strings/ascii.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {

Tracers::OpenTelemetry::SpanContext 
TypeConverter::toOpenTelemetrySpanContext(const SpanContext& generic_context) {
  return Tracers::OpenTelemetry::SpanContext(
    "00", // version - OpenTelemetry uses fixed version
    generic_context.traceId().toHex(),
    generic_context.spanId().toHex(),
    generic_context.sampled(),
    generic_context.tracestate()
  );
}

SpanContext 
TypeConverter::fromOpenTelemetrySpanContext(const Tracers::OpenTelemetry::SpanContext& otel_context) {
  TraceId trace_id(otel_context.traceId());
  SpanId span_id(otel_context.spanId());
  TraceFlags trace_flags = toTraceFlags(otel_context.sampled());
  
  return SpanContext(std::move(trace_id), std::move(span_id), trace_flags, 
                     absl::nullopt, otel_context.tracestate());
}

absl::optional<std::string> 
TypeConverter::extractParentSpanId(const SpanContext& generic_context) {
  if (generic_context.parentSpanId().has_value()) {
    return generic_context.parentSpanId()->toHex();
  }
  return absl::nullopt;
}

SpanContext 
TypeConverter::createSpanContextWithParent(const TraceId& trace_id, const SpanId& span_id, 
                                           const TraceFlags& trace_flags, const SpanId& parent_span_id,
                                           const std::string& tracestate) {
  return SpanContext(trace_id, span_id, trace_flags, parent_span_id, tracestate);
}

TraceId TypeConverter::toTraceId(absl::string_view trace_id_str) {
  return TraceId(trace_id_str);
}

SpanId TypeConverter::toSpanId(absl::string_view span_id_str) {
  return SpanId(span_id_str);
}

TraceFlags TypeConverter::toTraceFlags(bool sampled) {
  TraceFlags flags;
  flags.setSampled(sampled);
  return flags;
}

absl::optional<bool> TypeConverter::parseB3SamplingState(absl::string_view sampling_state) {
  if (sampling_state.empty()) {
    return absl::nullopt;
  }
  
  std::string lower_state = absl::AsciiStrToLower(sampling_state);
  
  if (lower_state == "1" || lower_state == "d") {
    return true;  // Sampled or debug (debug implies sampled)
  } else if (lower_state == "0") {
    return false; // Not sampled
  }
  
  return absl::nullopt; // Invalid sampling state
}

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
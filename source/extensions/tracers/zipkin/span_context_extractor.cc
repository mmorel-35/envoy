#include "source/extensions/tracers/zipkin/span_context_extractor.h"

#include "source/common/common/assert.h"
#include "source/common/common/utility.h"
#include "source/extensions/tracers/zipkin/span_context.h"
#include "source/extensions/tracers/zipkin/zipkin_core_constants.h"
#include "source/extensions/propagators/b3/propagator.h"
#include "source/extensions/propagators/w3c/propagator.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace Zipkin {

SpanContextExtractor::SpanContextExtractor(Tracing::TraceContext& trace_context,
                                           bool w3c_fallback_enabled)
    : trace_context_(trace_context), w3c_fallback_enabled_(w3c_fallback_enabled) {}

SpanContextExtractor::~SpanContextExtractor() = default;

absl::optional<bool> SpanContextExtractor::extractSampled() {
  // Try B3 format first using new propagator
  if (Extensions::Propagators::B3::Propagator::isPresent(trace_context_)) {
    auto b3_context = Extensions::Propagators::B3::TracingHelper::extractForTracer(trace_context_);
    if (b3_context.has_value()) {
      return Extensions::Propagators::B3::TracingHelper::isSampled(b3_context.value().getSamplingState());
    }
  }

  // Try W3C Trace Context format as fallback only if enabled
  if (w3c_fallback_enabled_ && Extensions::Propagators::W3C::Propagator::isPresent(trace_context_)) {
    auto w3c_context = Extensions::Propagators::W3C::TracingHelper::extractForTracer(trace_context_);
    if (w3c_context.has_value()) {
      return w3c_context.value().sampled;
    }
  }

  return absl::nullopt;
}

std::pair<SpanContext, bool> SpanContextExtractor::extractSpanContext(bool is_sampled) {
  // Try B3 format first using new propagator
  if (Extensions::Propagators::B3::Propagator::isPresent(trace_context_)) {
    auto b3_context = Extensions::Propagators::B3::TracingHelper::extractForTracer(trace_context_);
    if (b3_context.has_value()) {
      return convertB3ToZipkin(b3_context.value(), is_sampled);
    }
  }

  // Try W3C Trace Context format as fallback only if enabled
  if (w3c_fallback_enabled_ && Extensions::Propagators::W3C::Propagator::isPresent(trace_context_)) {
    auto w3c_result = Extensions::Propagators::W3C::Propagator::extract(trace_context_);
    if (w3c_result.ok()) {
      return convertW3CToZipkin(w3c_result.value(), is_sampled);
    }
  }

  return {SpanContext(), false};
}

std::pair<SpanContext, bool> SpanContextExtractor::convertB3ToZipkin(
    const Extensions::Propagators::B3::TraceContext& b3_context, bool fallback_sampled) {
  // B3 TraceContext already has the right format for Zipkin
  const auto& trace_id = b3_context.traceId();
  const auto& span_id = b3_context.spanId();
  const auto& parent_span_id = b3_context.parentSpanId();
  
  // Use B3 sampling decision, or fallback if not specified
  bool sampled = Extensions::Propagators::B3::TracingHelper::isSampled(b3_context.samplingState()) || fallback_sampled;
  
  uint64_t parent_id = parent_span_id.has_value() ? parent_span_id.value().value() : 0;
  
  return {SpanContext(trace_id.high(), trace_id.low(), span_id.value(), 
                     parent_id, sampled), true};
}

std::pair<SpanContext, bool> SpanContextExtractor::convertW3CToZipkin(
    const Extensions::Propagators::W3C::TraceContext& w3c_context, bool fallback_sampled) {
  // Convert W3C 128-bit trace ID (32 hex chars) to Zipkin format
  const auto& trace_parent = w3c_context.traceParent();
  const absl::string_view trace_id_str = trace_parent.traceId();

  if (trace_id_str.length() != 32) {
    throw ExtractorException(fmt::format("Invalid W3C trace ID length: {}", trace_id_str.length()));
  }

  // Split 128-bit trace ID into high and low 64-bit parts for Zipkin
  const absl::string_view trace_id_high_str = trace_id_str.substr(0, 16);
  const absl::string_view trace_id_low_str = trace_id_str.substr(16, 16);

  uint64_t trace_id_high(0);
  uint64_t trace_id(0);
  if (!StringUtil::atoull(std::string(trace_id_high_str).c_str(), trace_id_high, 16) ||
      !StringUtil::atoull(std::string(trace_id_low_str).c_str(), trace_id, 16)) {
    throw ExtractorException(fmt::format("Invalid W3C trace ID: {}", trace_id_str));
  }

  // Convert W3C span ID (16 hex chars) to Zipkin span ID
  const absl::string_view span_id_str = trace_parent.parentId();
  if (span_id_str.length() != 16) {
    throw ExtractorException(fmt::format("Invalid W3C span ID length: {}", span_id_str.length()));
  }

  uint64_t span_id(0);
  if (!StringUtil::atoull(std::string(span_id_str).c_str(), span_id, 16)) {
    throw ExtractorException(fmt::format("Invalid W3C span ID: {}", span_id_str));
  }

  // W3C doesn't have a direct parent span concept like B3
  // The W3C span-id becomes our span-id, and we don't set a parent
  uint64_t parent_id(0);

  // Use W3C sampling decision, or fallback if not specified
  bool sampled = trace_parent.isSampled() || fallback_sampled;

  return {SpanContext(trace_id_high, trace_id, span_id, parent_id, sampled), true};
}

} // namespace Zipkin
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

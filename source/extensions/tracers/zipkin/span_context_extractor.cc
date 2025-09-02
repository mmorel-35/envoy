#include "source/extensions/tracers/zipkin/span_context_extractor.h"

#include <charconv>

#include "source/common/common/assert.h"
#include "source/common/common/utility.h"
#include "source/extensions/propagators/propagator_factory.h"
#include "source/extensions/tracers/zipkin/span_context.h"
#include "source/extensions/tracers/zipkin/zipkin_core_constants.h"

#include "absl/strings/str_split.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace Zipkin {
namespace {

// Helper function to parse hex string_view to uint64_t using std::from_chars
bool parseHexStringView(absl::string_view hex_str, uint64_t& result) {
  const char* begin = hex_str.data();
  const char* end = begin + hex_str.size();
  auto [ptr, ec] = std::from_chars(begin, end, result, 16);
  return ec == std::errc{} && ptr == end;
}

// Convert OpenTelemetry SpanContext to Zipkin SpanContext
std::pair<SpanContext, bool> convertOtelToZipkin(const Extensions::Tracers::OpenTelemetry::SpanContext& otel_context,
                                                  const Tracing::TraceContext& trace_context) {
  // Convert trace ID from hex string to uint64_t values
  const absl::string_view trace_id_str = otel_context.traceId();
  
  if (trace_id_str.length() != 32) {
    throw ExtractorException(fmt::format("Invalid OpenTelemetry trace ID length: {}", trace_id_str.length()));
  }

  // Split 128-bit trace ID into high and low 64-bit parts for Zipkin
  const absl::string_view trace_id_high_str = absl::string_view(trace_id_str).substr(0, 16);
  const absl::string_view trace_id_low_str = absl::string_view(trace_id_str).substr(16, 16);

  uint64_t trace_id_high(0);
  uint64_t trace_id(0);
  if (!parseHexStringView(trace_id_high_str, trace_id_high) ||
      !parseHexStringView(trace_id_low_str, trace_id)) {
    throw ExtractorException(fmt::format("Invalid OpenTelemetry trace ID: {}", trace_id_str));
  }

  // Convert span ID from hex string to uint64_t
  const absl::string_view span_id_str = otel_context.spanId();
  if (span_id_str.length() != 16) {
    throw ExtractorException(fmt::format("Invalid OpenTelemetry span ID length: {}", span_id_str.length()));
  }

  uint64_t span_id(0);
  if (!parseHexStringView(span_id_str, span_id)) {
    throw ExtractorException(fmt::format("Invalid OpenTelemetry span ID: {}", span_id_str));
  }

  // Extract parent ID from B3 headers if present - this is Zipkin-specific functionality
  // that OpenTelemetry SpanContext doesn't support
  uint64_t parent_id(0);
  
  // Try B3 single header format for parent ID
  auto b3_header = Tracing::TraceContextHandler("b3").get(trace_context);
  if (b3_header.has_value()) {
    // Parse parent ID from B3 single header: trace-span-sampled-parent
    auto header_value = b3_header.value();
    std::vector<absl::string_view> parts = absl::StrSplit(header_value, '-');
    if (parts.size() == 4 && !parts[3].empty()) {
      // Fourth part is parent ID
      if (!parseHexStringView(parts[3], parent_id)) {
        parent_id = 0; // Invalid parent ID, set to 0
      }
    }
  } else {
    // Try multi-header format for parent ID
    auto parent_header = Tracing::TraceContextHandler("X-B3-ParentSpanId").get(trace_context);
    if (parent_header.has_value() && !parent_header.value().empty()) {
      if (!parseHexStringView(parent_header.value(), parent_id)) {
        parent_id = 0; // Invalid parent ID, set to 0
      }
    }
  }

  bool sampled = otel_context.sampled();
  return {SpanContext(trace_id_high, trace_id, span_id, parent_id, sampled), true};
}

} // namespace

SpanContextExtractor::SpanContextExtractor(Tracing::TraceContext& trace_context,
                                           bool w3c_fallback_enabled)
    : trace_context_(trace_context), w3c_fallback_enabled_(w3c_fallback_enabled) {}

SpanContextExtractor::SpanContextExtractor(Tracing::TraceContext& trace_context,
                                           bool w3c_fallback_enabled,
                                           const std::vector<std::string>& w3c_propagator_names)
    : trace_context_(trace_context), w3c_fallback_enabled_(w3c_fallback_enabled), 
      w3c_propagator_names_(w3c_propagator_names) {}

SpanContextExtractor::~SpanContextExtractor() = default;

absl::optional<bool> SpanContextExtractor::extractSampled() {
  // Try B3 propagation first using shared propagator
  auto b3_propagator = Extensions::Propagators::PropagatorFactory::createPropagators({"b3"});
  if (b3_propagator->propagationHeaderPresent(trace_context_)) {
    auto b3_result = b3_propagator->extract(trace_context_);
    if (b3_result.ok()) {
      return b3_result.value().sampled();
    } else {
      // Handle special B3 cases that shared propagator treats as errors
      // but still contain sampling information
      auto b3_header = Tracing::TraceContextHandler("b3").get(trace_context_);
      if (b3_header.has_value()) {
        auto header_value = b3_header.value();
        if (header_value == "0") {
          return false; // Explicitly not sampled
        } else if (header_value == "1" || header_value == "d") {
          return true; // Debug or explicit sampling
        }
      }
    }
  }

  // Try W3C Trace Context format as fallback only if enabled.
  if (w3c_fallback_enabled_) {
    // Use configured propagator names if provided, otherwise use default W3C propagator.
    // This follows OpenTelemetry specification for propagator configuration:
    // - When propagator names are provided: use PropagatorFactory::createPropagators() 
    // - When empty (default): use PropagatorFactory::createDefaultPropagators()
    // This eliminates ad-hoc propagator creation and enables future configuration support.
    auto w3c_propagator = w3c_propagator_names_.empty() 
        ? Extensions::Propagators::PropagatorFactory::createDefaultPropagators()
        : Extensions::Propagators::PropagatorFactory::createPropagators(w3c_propagator_names_);
    Extensions::Tracers::OpenTelemetry::SpanContextExtractor w3c_extractor(
        const_cast<Tracing::TraceContext&>(trace_context_), std::move(w3c_propagator));
    if (w3c_extractor.propagationHeaderPresent()) {
      auto w3c_span_context = w3c_extractor.extractSpanContext();
      if (w3c_span_context.ok()) {
        return w3c_span_context.value().sampled();
      }
    }
  }

  return absl::nullopt;
}

std::pair<SpanContext, bool> SpanContextExtractor::extractSpanContext(bool is_sampled) {
  // Try B3 propagation first using shared propagator
  auto b3_propagator = Extensions::Propagators::PropagatorFactory::createPropagators({"b3"});
  if (b3_propagator->propagationHeaderPresent(trace_context_)) {
    auto b3_result = b3_propagator->extract(trace_context_);
    if (b3_result.ok()) {
      return convertOtelToZipkin(b3_result.value(), trace_context_);
    }
  }

  // Try W3C Trace Context format as fallback only if enabled.
  if (w3c_fallback_enabled_) {
    // Use configured propagator names if provided, otherwise use default W3C propagator.
    // This follows OpenTelemetry specification for propagator configuration:
    // - When propagator names are provided: use PropagatorFactory::createPropagators() 
    // - When empty (default): use PropagatorFactory::createDefaultPropagators()
    // This eliminates ad-hoc propagator creation and enables future configuration support.
    auto w3c_propagator = w3c_propagator_names_.empty() 
        ? Extensions::Propagators::PropagatorFactory::createDefaultPropagators()
        : Extensions::Propagators::PropagatorFactory::createPropagators(w3c_propagator_names_);
    Extensions::Tracers::OpenTelemetry::SpanContextExtractor w3c_extractor(
        const_cast<Tracing::TraceContext&>(trace_context_), std::move(w3c_propagator));
    if (w3c_extractor.propagationHeaderPresent()) {
      auto w3c_span_context = w3c_extractor.extractSpanContext();
      if (w3c_span_context.ok()) {
        return convertW3CToZipkin(w3c_span_context.value(), is_sampled);
      }
    }
  }

  return {SpanContext(), false};
}

std::pair<SpanContext, bool> SpanContextExtractor::convertW3CToZipkin(
    const Extensions::Tracers::OpenTelemetry::SpanContext& w3c_context, bool fallback_sampled) {
  // Convert W3C 128-bit trace ID (32 hex chars) to Zipkin format.
  const absl::string_view trace_id_str = w3c_context.traceId();

  if (trace_id_str.length() != 32) {
    throw ExtractorException(fmt::format("Invalid W3C trace ID length: {}", trace_id_str.length()));
  }

  // Split 128-bit trace ID into high and low 64-bit parts for Zipkin.
  const absl::string_view trace_id_high_str = absl::string_view(trace_id_str).substr(0, 16);
  const absl::string_view trace_id_low_str = absl::string_view(trace_id_str).substr(16, 16);

  uint64_t trace_id_high(0);
  uint64_t trace_id(0);
  if (!parseHexStringView(trace_id_high_str, trace_id_high) ||
      !parseHexStringView(trace_id_low_str, trace_id)) {
    throw ExtractorException(fmt::format("Invalid W3C trace ID: {}", trace_id_str));
  }

  // Convert W3C span ID (16 hex chars) to Zipkin span ID.
  const absl::string_view span_id_str = w3c_context.spanId();
  if (span_id_str.length() != 16) {
    throw ExtractorException(fmt::format("Invalid W3C span ID length: {}", span_id_str.length()));
  }

  uint64_t span_id(0);
  if (!parseHexStringView(span_id_str, span_id)) {
    throw ExtractorException(fmt::format("Invalid W3C span ID: {}", span_id_str));
  }

  // W3C doesn't have a direct parent span concept like B3
  // The W3C span-id becomes our span-id, and we don't set a parent.
  uint64_t parent_id(0);

  // Use W3C sampling decision, or fallback if not specified.
  bool sampled = w3c_context.sampled() || fallback_sampled;

  return {SpanContext(trace_id_high, trace_id, span_id, parent_id, sampled), true};
}

} // namespace Zipkin
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

#include "source/extensions/tracers/opentelemetry/span_context_extractor.h"

#include "envoy/tracing/tracer.h"

#include "source/common/http/header_map_impl.h"
#include "source/common/tracing/trace_context_impl.h"
#include "source/extensions/propagators/opentelemetry/propagator.h"
#include "source/extensions/tracers/opentelemetry/span_context.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

SpanContextExtractor::SpanContextExtractor(Tracing::TraceContext& trace_context)
    : trace_context_(trace_context) {}

SpanContextExtractor::~SpanContextExtractor() = default;

bool SpanContextExtractor::propagationHeaderPresent() {
  return Extensions::Propagators::OpenTelemetry::Propagator::isPresent(trace_context_);
}

absl::StatusOr<SpanContext> SpanContextExtractor::extractSpanContext() {
  // Use the OpenTelemetry composite propagator for multi-format extraction
  auto composite_result = Extensions::Propagators::OpenTelemetry::TracingHelper::extractForTracer(trace_context_);
  if (!composite_result.has_value()) {
    return absl::InvalidArgumentError("No supported propagation headers found");
  }

  return convertCompositeToOtel(composite_result.value());
}

absl::StatusOr<SpanContext> SpanContextExtractor::convertCompositeToOtel(
    const Extensions::Propagators::OpenTelemetry::CompositeTraceContext& composite_context) {
  
  // Get trace information from composite context
  const std::string trace_id = composite_context.getTraceId();
  const std::string span_id = composite_context.getSpanId();
  const bool sampled = composite_context.isSampled();
  
  // Get trace state if available (only for W3C format)
  std::string trace_state;
  if (composite_context.format() == Extensions::Propagators::OpenTelemetry::TraceFormat::W3C) {
    // Extract trace state from W3C context
    const auto& w3c_context = composite_context.getW3CContext();
    if (w3c_context.has_value()) {
      const auto& trace_state_obj = w3c_context.value().traceState();
      trace_state = trace_state_obj.toString();
    }
  }
  
  // Create OpenTelemetry SpanContext with version "00" for compatibility
  SpanContext parent_context("00", trace_id, span_id, sampled, trace_state);
  return parent_context;
}

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

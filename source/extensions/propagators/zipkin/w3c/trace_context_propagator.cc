#include "source/extensions/propagators/zipkin/w3c/trace_context_propagator.h"

#include "source/common/common/hex.h"
#include "source/extensions/propagators/trace_context.h"
#include "source/extensions/tracers/common/utils/trace.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace Zipkin {

absl::StatusOr<Extensions::Tracers::Zipkin::SpanContext>
W3CTraceContextPropagator::extract(const Tracing::TraceContext& trace_context) {
  // Use the base W3C propagator to extract generic span context
  auto generic_result = base_propagator_.extract(trace_context);
  if (!generic_result.ok()) {
    return generic_result.status();
  }

  // Convert generic span context to Zipkin span context
  return convertFromGeneric(generic_result.value());
}

void W3CTraceContextPropagator::inject(const Extensions::Tracers::Zipkin::SpanContext& span_context,
                                       Tracing::TraceContext& trace_context) {
  // Convert Zipkin span context to generic span context
  auto generic_span_context = convertToGeneric(span_context);
  
  // Use the base W3C propagator to inject
  base_propagator_.inject(generic_span_context, trace_context);
}

std::vector<std::string> W3CTraceContextPropagator::fields() const {
  return base_propagator_.fields();
}

Extensions::Tracers::Zipkin::SpanContext W3CTraceContextPropagator::convertFromGeneric(const Extensions::Propagators::SpanContext& generic_span_context) {
  // Convert from generic SpanContext to Zipkin SpanContext
  const std::string& trace_id_hex = generic_span_context.traceId().toHex();
  const std::string& span_id_hex = generic_span_context.spanId().toHex();
  
  uint64_t trace_id_high = 0, trace_id_low = 0;
  Extensions::Tracers::Common::Utils::Trace::parseTraceId(trace_id_hex, trace_id_high, trace_id_low);
  
  uint64_t span_id = 0;
  Extensions::Tracers::Common::Utils::Trace::parseSpanId(span_id_hex, span_id);
  
  uint64_t parent_span_id = 0;
  if (generic_span_context.parentSpanId().has_value()) {
    const std::string& parent_span_id_hex = generic_span_context.parentSpanId().value().toHex();
    Extensions::Tracers::Common::Utils::Trace::parseSpanId(parent_span_id_hex, parent_span_id);
  }
  
  return Extensions::Tracers::Zipkin::SpanContext(
    trace_id_high, 
    trace_id_low, 
    span_id, 
    parent_span_id, 
    generic_span_context.sampled()
  );
}

Extensions::Propagators::SpanContext W3CTraceContextPropagator::convertToGeneric(const Extensions::Tracers::Zipkin::SpanContext& zipkin_span_context) {
  // Convert from Zipkin SpanContext to generic SpanContext
  std::string trace_id_hex;
  if (zipkin_span_context.is128BitTraceId()) {
    trace_id_hex = Hex::uint64ToHex(zipkin_span_context.traceIdHigh()) + 
                   Hex::uint64ToHex(zipkin_span_context.traceId());
  } else {
    // For 64-bit trace IDs, pad with zeros to make it 128-bit for W3C compliance
    trace_id_hex = std::string(16, '0') + Hex::uint64ToHex(zipkin_span_context.traceId());
  }
  
  std::string span_id_hex = Hex::uint64ToHex(zipkin_span_context.id());
  
  Extensions::Propagators::TraceId trace_id(trace_id_hex);
  Extensions::Propagators::SpanId span_id(span_id_hex);
  Extensions::Propagators::TraceFlags trace_flags;
  trace_flags.setSampled(zipkin_span_context.sampled());
  
  absl::optional<Extensions::Propagators::SpanId> parent_span_id;
  if (zipkin_span_context.parentId() != 0) {
    std::string parent_span_id_hex = Hex::uint64ToHex(zipkin_span_context.parentId());
    parent_span_id = Extensions::Propagators::SpanId(parent_span_id_hex);
  }
  
  return Extensions::Propagators::SpanContext(
    trace_id, 
    span_id, 
    trace_flags, 
    parent_span_id, 
    "" // tracestate not used in Zipkin
  );
}

} // namespace Zipkin
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

} // namespace Zipkin
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

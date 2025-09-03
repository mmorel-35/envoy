#include "source/extensions/propagators/opentelemetry/trace_context.h"

#include "source/extensions/propagators/w3c/propagator.h"
#include "source/extensions/propagators/b3/propagator.h"

#include "absl/strings/str_format.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace OpenTelemetry {

// CompositeTraceContext implementation

CompositeTraceContext::CompositeTraceContext(const W3C::TraceContext& w3c_context)
    : format_(TraceFormat::W3C), context_(w3c_context) {}

CompositeTraceContext::CompositeTraceContext(const B3::TraceContext& b3_context)
    : format_(TraceFormat::B3), context_(b3_context) {}

absl::optional<W3C::TraceContext> CompositeTraceContext::getW3CContext() const {
  if (format_ == TraceFormat::W3C) {
    return absl::get<W3C::TraceContext>(context_);
  }
  return absl::nullopt;
}

absl::optional<B3::TraceContext> CompositeTraceContext::getB3Context() const {
  if (format_ == TraceFormat::B3) {
    return absl::get<B3::TraceContext>(context_);
  }
  return absl::nullopt;
}

std::string CompositeTraceContext::getTraceId() const {
  switch (format_) {
    case TraceFormat::W3C: {
      const auto& w3c_ctx = absl::get<W3C::TraceContext>(context_);
      return w3c_ctx.traceparent().trace_id();
    }
    case TraceFormat::B3: {
      const auto& b3_ctx = absl::get<B3::TraceContext>(context_);
      return b3_ctx.traceId().toHexString();
    }
    case TraceFormat::NONE:
    default:
      return "";
  }
}

std::string CompositeTraceContext::getSpanId() const {
  switch (format_) {
    case TraceFormat::W3C: {
      const auto& w3c_ctx = absl::get<W3C::TraceContext>(context_);
      return w3c_ctx.traceparent().span_id();
    }
    case TraceFormat::B3: {
      const auto& b3_ctx = absl::get<B3::TraceContext>(context_);
      return b3_ctx.spanId().toHexString();
    }
    case TraceFormat::NONE:
    default:
      return "";
  }
}

std::string CompositeTraceContext::getParentSpanId() const {
  switch (format_) {
    case TraceFormat::W3C: {
      const auto& w3c_ctx = absl::get<W3C::TraceContext>(context_);
      return w3c_ctx.traceparent().parent_id();
    }
    case TraceFormat::B3: {
      const auto& b3_ctx = absl::get<B3::TraceContext>(context_);
      if (b3_ctx.parentSpanId().has_value()) {
        return b3_ctx.parentSpanId().value().toHexString();
      }
      return "";
    }
    case TraceFormat::NONE:
    default:
      return "";
  }
}

bool CompositeTraceContext::isSampled() const {
  switch (format_) {
    case TraceFormat::W3C: {
      const auto& w3c_ctx = absl::get<W3C::TraceContext>(context_);
      return w3c_ctx.traceparent().sampled();
    }
    case TraceFormat::B3: {
      const auto& b3_ctx = absl::get<B3::TraceContext>(context_);
      return B3::TracingHelper::isSampled(b3_ctx.samplingState());
    }
    case TraceFormat::NONE:
    default:
      return false;
  }
}

std::string CompositeTraceContext::getTraceState() const {
  switch (format_) {
    case TraceFormat::W3C: {
      const auto& w3c_ctx = absl::get<W3C::TraceContext>(context_);
      if (w3c_ctx.tracestate().has_value()) {
        return w3c_ctx.tracestate().value().toString();
      }
      return "";
    }
    case TraceFormat::B3:
    case TraceFormat::NONE:
    default:
      return ""; // B3 doesn't have tracestate
  }
}

absl::StatusOr<CompositeTraceContext> CompositeTraceContext::createChild(absl::string_view new_span_id) const {
  switch (format_) {
    case TraceFormat::W3C: {
      const auto& w3c_ctx = absl::get<W3C::TraceContext>(context_);
      auto child_result = W3C::Propagator::createChild(w3c_ctx, new_span_id);
      if (!child_result.ok()) {
        return child_result.status();
      }
      return CompositeTraceContext(child_result.value());
    }
    case TraceFormat::B3: {
      const auto& b3_ctx = absl::get<B3::TraceContext>(context_);
      // For B3, create child by setting parent span ID to current span ID
      auto child_ctx = b3_ctx;
      
      // Parse new span ID
      auto span_id_result = B3::SpanId::fromHexString(new_span_id);
      if (!span_id_result.ok()) {
        return span_id_result.status();
      }
      
      // Set parent span ID to current span ID, and update span ID
      child_ctx.setParentSpanId(b3_ctx.spanId());
      child_ctx.setSpanId(span_id_result.value());
      
      return CompositeTraceContext(child_ctx);
    }
    case TraceFormat::NONE:
    default:
      return absl::InvalidArgumentError("Cannot create child from invalid trace context");
  }
}

absl::StatusOr<CompositeTraceContext> CompositeTraceContext::convertTo(TraceFormat target_format) const {
  if (format_ == target_format) {
    return *this; // Already in target format
  }
  
  if (format_ == TraceFormat::NONE) {
    return absl::InvalidArgumentError("Cannot convert empty trace context");
  }
  
  // Get common values from current format
  std::string trace_id = getTraceId();
  std::string span_id = getSpanId();
  std::string parent_span_id = getParentSpanId();
  bool sampled = isSampled();
  
  switch (target_format) {
    case TraceFormat::W3C: {
      // Convert to W3C format
      auto w3c_result = W3C::Propagator::createRoot(trace_id, span_id, sampled);
      if (!w3c_result.ok()) {
        return w3c_result.status();
      }
      
      // If we have parent span ID, create as child instead
      if (!parent_span_id.empty()) {
        // For W3C, parent_id in traceparent is the parent span ID
        auto w3c_ctx = w3c_result.value();
        w3c_ctx.mutableTraceparent().setParentId(parent_span_id);
        return CompositeTraceContext(w3c_ctx);
      }
      
      return CompositeTraceContext(w3c_result.value());
    }
    case TraceFormat::B3: {
      // Convert to B3 format
      // Parse trace ID (handle both 128-bit and 64-bit)
      auto trace_id_result = B3::TraceId::fromHexString(trace_id);
      if (!trace_id_result.ok()) {
        return trace_id_result.status();
      }
      
      auto span_id_result = B3::SpanId::fromHexString(span_id);
      if (!span_id_result.ok()) {
        return span_id_result.status();
      }
      
      absl::optional<B3::SpanId> parent_span_id_opt;
      if (!parent_span_id.empty()) {
        auto parent_result = B3::SpanId::fromHexString(parent_span_id);
        if (!parent_result.ok()) {
          return parent_result.status();
        }
        parent_span_id_opt = parent_result.value();
      }
      
      B3::SamplingState sampling_state = sampled ? B3::SamplingState::SAMPLED : B3::SamplingState::NOT_SAMPLED;
      
      B3::TraceContext b3_ctx(trace_id_result.value(), span_id_result.value(), 
                             parent_span_id_opt, sampling_state);
      
      return CompositeTraceContext(b3_ctx);
    }
    case TraceFormat::NONE:
    default:
      return absl::InvalidArgumentError("Invalid target format for conversion");
  }
}

// CompositeBaggage implementation

CompositeBaggage::CompositeBaggage(const W3C::Baggage& w3c_baggage)
    : w3c_baggage_(w3c_baggage) {}

bool CompositeBaggage::isEmpty() const {
  if (w3c_baggage_.has_value()) {
    return w3c_baggage_.value().empty();
  }
  return true;
}

std::string CompositeBaggage::getValue(absl::string_view key) const {
  if (w3c_baggage_.has_value()) {
    auto member = w3c_baggage_.value().get(key);
    if (member.has_value()) {
      return member.value().value();
    }
  }
  return "";
}

bool CompositeBaggage::setValue(absl::string_view key, absl::string_view value) {
  if (!w3c_baggage_.has_value()) {
    w3c_baggage_ = W3C::Baggage(); // Create empty baggage
  }
  
  W3C::BaggageMember member(key, value);
  auto result = w3c_baggage_.value().set(member);
  if (result.ok()) {
    w3c_baggage_ = result.value();
    return true;
  }
  return false;
}

std::map<std::string, std::string> CompositeBaggage::getAllEntries() const {
  std::map<std::string, std::string> result;
  
  if (w3c_baggage_.has_value()) {
    for (const auto& member : w3c_baggage_.value().members()) {
      result[member.key()] = member.value();
    }
  }
  
  return result;
}

absl::optional<W3C::Baggage> CompositeBaggage::getW3CBaggage() const {
  return w3c_baggage_;
}

} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
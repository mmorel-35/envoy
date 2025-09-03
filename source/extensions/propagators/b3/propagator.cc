#include "source/extensions/propagators/b3/propagator.h"

#include "source/common/common/utility.h"
#include "source/common/tracing/trace_context_impl.h"

#include "absl/status/status.h" 
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace B3 {

namespace {

// B3 header names
constexpr char kB3TraceIdHeader[] = "x-b3-traceid";
constexpr char kB3SpanIdHeader[] = "x-b3-spanid";
constexpr char kB3ParentSpanIdHeader[] = "x-b3-parentspanid";
constexpr char kB3SampledHeader[] = "x-b3-sampled";
constexpr char kB3FlagsHeader[] = "x-b3-flags";
constexpr char kB3SingleHeader[] = "b3";

// Trace context handlers for B3 headers
const Tracing::TraceContextHandler B3_TRACE_ID{kB3TraceIdHeader};
const Tracing::TraceContextHandler B3_SPAN_ID{kB3SpanIdHeader};
const Tracing::TraceContextHandler B3_PARENT_SPAN_ID{kB3ParentSpanIdHeader};
const Tracing::TraceContextHandler B3_SAMPLED{kB3SampledHeader};
const Tracing::TraceContextHandler B3_FLAGS{kB3FlagsHeader};
const Tracing::TraceContextHandler B3_SINGLE{kB3SingleHeader};

/**
 * Extracts the first value from a header, as headers are implicitly untrusted.
 */
absl::optional<std::string> getHeaderValue(const Tracing::TraceContextHandler& handler,
                                          const Tracing::TraceContext& trace_context) {
  auto header_entry = handler.get(trace_context);
  if (header_entry.has_value()) {
    return std::string(header_entry.value());
  }
  return absl::nullopt;
}

/**
 * Sets a header value in the trace context.
 */
void setHeaderValue(const Tracing::TraceContextHandler& handler,
                   const std::string& value,
                   Tracing::TraceContext& trace_context) {
  handler.set(trace_context, value);
}

} // namespace

// Propagator implementation

bool Propagator::isPresent(const Tracing::TraceContext& trace_context) {
  return B3_SINGLE.get(trace_context).has_value() ||
         B3_TRACE_ID.get(trace_context).has_value() ||
         B3_SPAN_ID.get(trace_context).has_value();
}

absl::StatusOr<TraceContext> Propagator::extract(const Tracing::TraceContext& trace_context) {
  // Try single header format first
  auto single_header = getHeaderValue(B3_SINGLE, trace_context);
  if (single_header.has_value()) {
    return extractSingleHeader(trace_context);
  }

  // Try multiple headers format
  auto trace_id_header = getHeaderValue(B3_TRACE_ID, trace_context);
  auto span_id_header = getHeaderValue(B3_SPAN_ID, trace_context);
  if (trace_id_header.has_value() && span_id_header.has_value()) {
    return extractMultipleHeaders(trace_context);
  }

  return absl::NotFoundError("No B3 headers found");
}

absl::Status Propagator::inject(const TraceContext& b3_context,
                               Tracing::TraceContext& trace_context) {
  // Use multiple headers format for maximum compatibility
  return injectMultipleHeaders(b3_context, trace_context);
}

absl::StatusOr<TraceContext> Propagator::extractSingleHeader(
    const Tracing::TraceContext& trace_context) {
  auto header_value = getHeaderValue(B3_SINGLE, trace_context);
  if (!header_value.has_value()) {
    return absl::NotFoundError("B3 single header not found");
  }

  const std::string& b3_value = header_value.value();
  if (b3_value.empty()) {
    return absl::InvalidArgumentError("B3 single header is empty");
  }

  // Handle single character sampling flags
  if (b3_value.size() == 1) {
    SamplingState sampling_state = parseSamplingState(b3_value);
    if (sampling_state == SamplingState::UNSPECIFIED) {
      return absl::InvalidArgumentError(
          absl::StrCat("Invalid B3 sampling flag: ", b3_value));
    }
    // Return empty trace context with just sampling state
    TraceContext context;
    context.setSamplingState(sampling_state);
    return context;
  }

  // Parse full format: {traceid}-{spanid}-{sampled}-{parentspanid}
  std::vector<absl::string_view> parts = absl::StrSplit(b3_value, '-');
  
  if (parts.size() < 2) {
    return absl::InvalidArgumentError("B3 single header format invalid: missing required fields");
  }

  // Parse trace ID
  auto trace_id = TraceId::fromHexString(parts[0]);
  if (!trace_id.ok()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid trace ID in B3 header: ", trace_id.status().message()));
  }

  // Parse span ID
  auto span_id = SpanId::fromHexString(parts[1]);
  if (!span_id.ok()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid span ID in B3 header: ", span_id.status().message()));
  }

  TraceContext context(trace_id.value(), span_id.value());

  // Parse sampling state if present
  if (parts.size() > 2 && !parts[2].empty()) {
    context.setSamplingState(parseSamplingState(parts[2]));
  }

  // Parse parent span ID if present
  if (parts.size() > 3 && !parts[3].empty()) {
    auto parent_span_id = SpanId::fromHexString(parts[3]);
    if (!parent_span_id.ok()) {
      return absl::InvalidArgumentError(
          absl::StrCat("Invalid parent span ID in B3 header: ", parent_span_id.status().message()));
    }
    context.setParentSpanId(parent_span_id.value());
  }

  return context;
}

absl::Status Propagator::injectSingleHeader(const TraceContext& b3_context,
                                           Tracing::TraceContext& trace_context) {
  auto header_value = b3_context.toSingleHeader();
  if (!header_value.ok()) {
    return header_value.status();
  }

  setHeaderValue(B3_SINGLE, header_value.value(), trace_context);
  return absl::OkStatus();
}

absl::StatusOr<TraceContext> Propagator::extractMultipleHeaders(
    const Tracing::TraceContext& trace_context) {
  auto trace_id_header = getHeaderValue(B3_TRACE_ID, trace_context);
  auto span_id_header = getHeaderValue(B3_SPAN_ID, trace_context);

  if (!trace_id_header.has_value() || !span_id_header.has_value()) {
    return absl::NotFoundError("Required B3 headers (trace ID or span ID) not found");
  }

  // Parse trace ID
  auto trace_id = TraceId::fromHexString(trace_id_header.value());
  if (!trace_id.ok()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid trace ID: ", trace_id.status().message()));
  }

  // Parse span ID  
  auto span_id = SpanId::fromHexString(span_id_header.value());
  if (!span_id.ok()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Invalid span ID: ", span_id.status().message()));
  }

  TraceContext context(trace_id.value(), span_id.value());

  // Parse parent span ID if present
  auto parent_span_id_header = getHeaderValue(B3_PARENT_SPAN_ID, trace_context);
  if (parent_span_id_header.has_value() && !parent_span_id_header.value().empty()) {
    auto parent_span_id = SpanId::fromHexString(parent_span_id_header.value());
    if (!parent_span_id.ok()) {
      return absl::InvalidArgumentError(
          absl::StrCat("Invalid parent span ID: ", parent_span_id.status().message()));
    }
    context.setParentSpanId(parent_span_id.value());
  }

  // Parse sampling state from sampled header
  auto sampled_header = getHeaderValue(B3_SAMPLED, trace_context);
  if (sampled_header.has_value()) {
    context.setSamplingState(parseSamplingState(sampled_header.value()));
  }

  // Check for debug flag
  auto flags_header = getHeaderValue(B3_FLAGS, trace_context);
  if (flags_header.has_value() && flags_header.value() == "1") {
    context.setDebug(true);
    context.setSamplingState(SamplingState::DEBUG);
  }

  return context;
}

absl::Status Propagator::injectMultipleHeaders(const TraceContext& b3_context,
                                              Tracing::TraceContext& trace_context) {
  if (!b3_context.isValid()) {
    return absl::InvalidArgumentError("Invalid B3 context: missing trace ID or span ID");
  }

  // Inject required headers
  setHeaderValue(B3_TRACE_ID, b3_context.traceId().toHexString(), trace_context);
  setHeaderValue(B3_SPAN_ID, b3_context.spanId().toHexString(), trace_context);

  // Inject parent span ID if present
  if (b3_context.parentSpanId().has_value()) {
    setHeaderValue(B3_PARENT_SPAN_ID, b3_context.parentSpanId().value().toHexString(), 
                   trace_context);
  }

  // Inject sampling state if specified
  if (b3_context.samplingState() != SamplingState::UNSPECIFIED) {
    setHeaderValue(B3_SAMPLED, samplingStateToString(b3_context.samplingState()), 
                   trace_context);
  }

  // Inject debug flag if set
  if (b3_context.debug()) {
    setHeaderValue(B3_FLAGS, "1", trace_context);
  }

  return absl::OkStatus();
}

SamplingState Propagator::parseSamplingState(absl::string_view value) {
  if (value == "1" || value == "true") {
    return SamplingState::SAMPLED;
  } else if (value == "0" || value == "false") {
    return SamplingState::NOT_SAMPLED;
  } else if (value == "d") {
    return SamplingState::DEBUG;
  } else {
    return SamplingState::UNSPECIFIED;
  }
}

std::string Propagator::samplingStateToString(SamplingState state) {
  switch (state) {
    case SamplingState::NOT_SAMPLED:
      return "0";
    case SamplingState::SAMPLED:
      return "1";
    case SamplingState::DEBUG:
      return "d";
    case SamplingState::UNSPECIFIED:
      return "";
  }
  return "";
}

// TracingHelper implementation

absl::optional<TraceContext> TracingHelper::extractForTracer(
    const Tracing::TraceContext& trace_context) {
  auto result = Propagator::extract(trace_context);
  if (result.ok()) {
    return result.value();
  }
  return absl::nullopt;
}

absl::Status TracingHelper::injectFromTracer(const TraceContext& b3_context,
                                            Tracing::TraceContext& trace_context) {
  return Propagator::inject(b3_context, trace_context);
}

bool TracingHelper::isSampled(SamplingState sampling_state) {
  return sampling_state == SamplingState::SAMPLED || 
         sampling_state == SamplingState::DEBUG;
}

TraceContext TracingHelper::createTraceContext(uint64_t trace_id_high, uint64_t trace_id_low,
                                              uint64_t span_id, uint64_t parent_span_id,
                                              bool sampled) {
  TraceId trace_id = (trace_id_high == 0) ? 
      TraceId::from64Bit(trace_id_low) : 
      TraceId::from128Bit(trace_id_high, trace_id_low);
  
  SpanId span_id_obj = SpanId::from64Bit(span_id);
  
  absl::optional<SpanId> parent_span_id_obj;
  if (parent_span_id != 0) {
    parent_span_id_obj = SpanId::from64Bit(parent_span_id);
  }

  SamplingState sampling_state = sampled ? SamplingState::SAMPLED : SamplingState::NOT_SAMPLED;

  return TraceContext(trace_id, span_id_obj, parent_span_id_obj, sampling_state);
}

} // namespace B3
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
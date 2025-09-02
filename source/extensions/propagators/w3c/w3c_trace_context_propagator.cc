#include "source/extensions/propagators/w3c/w3c_trace_context_propagator.h"

#include "source/common/common/hex.h"
#include "source/common/tracing/trace_context_impl.h"

#include "absl/strings/escaping.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace {

constexpr absl::string_view kDefaultVersion = "00";

// See https://www.w3.org/TR/trace-context/#traceparent-header
constexpr int kTraceparentHeaderSize = 55; // 2 + 1 + 32 + 1 + 16 + 1 + 2
constexpr int kVersionHexSize = 2;
constexpr int kTraceIdHexSize = 32;
constexpr int kParentIdHexSize = 16;
constexpr int kTraceFlagsHexSize = 2;

bool isValidHex(const absl::string_view& input) {
  return std::all_of(input.begin(), input.end(),
                     [](const char& c) { return absl::ascii_isxdigit(c); });
}

bool isAllZeros(const absl::string_view& input) {
  return std::all_of(input.begin(), input.end(), [](const char& c) { return c == '0'; });
}

} // namespace

W3CTraceContextPropagator::W3CTraceContextPropagator()
    : trace_parent_header_("traceparent"), trace_state_header_("tracestate") {}

absl::StatusOr<Tracers::OpenTelemetry::SpanContext>
W3CTraceContextPropagator::extract(const Tracing::TraceContext& trace_context) {
  auto propagation_header = trace_parent_header_.get(trace_context);
  if (!propagation_header.has_value()) {
    return absl::InvalidArgumentError("No traceparent header found");
  }
  auto header_value_string = propagation_header.value();

  if (header_value_string.size() != kTraceparentHeaderSize) {
    return absl::InvalidArgumentError("Invalid traceparent header length");
  }

  // Parse: version-trace_id-parent_id-trace_flags
  std::vector<absl::string_view> parts = absl::StrSplit(header_value_string, '-');
  if (parts.size() != 4) {
    return absl::InvalidArgumentError("Invalid traceparent header format");
  }

  absl::string_view version = parts[0];
  absl::string_view trace_id = parts[1];
  absl::string_view parent_id = parts[2];
  absl::string_view trace_flags = parts[3];

  // Validate format
  if (version.size() != kVersionHexSize || !isValidHex(version) ||
      trace_id.size() != kTraceIdHexSize || !isValidHex(trace_id) || isAllZeros(trace_id) ||
      parent_id.size() != kParentIdHexSize || !isValidHex(parent_id) || isAllZeros(parent_id) ||
      trace_flags.size() != kTraceFlagsHexSize || !isValidHex(trace_flags)) {
    return absl::InvalidArgumentError("Invalid traceparent header values");
  }

  // Parse trace flags for sampling decision
  uint8_t flags;
  if (!absl::SimpleHexAtoi(trace_flags, &flags)) {
    return absl::InvalidArgumentError("Invalid trace flags");
  }
  bool sampled = (flags & 0x01) != 0;

  // Get trace state
  std::string trace_state = "";
  auto trace_state_header = trace_state_header_.get(trace_context);
  if (trace_state_header.has_value()) {
    trace_state = std::string(trace_state_header.value());
  }

  Tracers::OpenTelemetry::SpanContext span_context(version, trace_id, parent_id, sampled, trace_state);
  return span_context;
}

void W3CTraceContextPropagator::inject(const Tracers::OpenTelemetry::SpanContext& span_context,
                                       Tracing::TraceContext& trace_context) {
  const std::string& version = span_context.version();
  const std::string& trace_id = span_context.traceId();
  const std::string& span_id = span_context.spanId();
  bool sampled = span_context.sampled();

  // Format trace flags
  uint8_t flags = sampled ? 0x01 : 0x00;
  std::string trace_flags = absl::StrCat(absl::Hex(flags, absl::kZeroPad2));

  // Format traceparent header
  std::string traceparent = absl::StrCat(version, "-", trace_id, "-", span_id, "-", trace_flags);
  trace_parent_header_.setRefKey(trace_context, traceparent);

  // Inject trace state if present
  const std::string& trace_state = span_context.traceState();
  if (!trace_state.empty()) {
    trace_state_header_.setRefKey(trace_context, trace_state);
  }
}

std::vector<std::string> W3CTraceContextPropagator::fields() const {
  return {"traceparent", "tracestate"};
}

std::string W3CTraceContextPropagator::name() const { return "tracecontext"; }

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
#include "source/extensions/propagators/w3c/baggage_propagator.h"

#include "source/common/tracing/trace_context_impl.h"

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {

BaggagePropagator::BaggagePropagator() : baggage_header_("baggage") {}

absl::StatusOr<Tracers::OpenTelemetry::SpanContext>
BaggagePropagator::extract(const Tracing::TraceContext& trace_context) {
  // Baggage propagator doesn't extract span context, only baggage
  // This would normally be handled by storing baggage in a context map
  // For now, return an error to indicate this propagator doesn't handle trace context
  return absl::InvalidArgumentError("Baggage propagator does not extract trace context");
}

void BaggagePropagator::inject(const Tracers::OpenTelemetry::SpanContext& span_context,
                               Tracing::TraceContext& trace_context) {
  // This would normally inject baggage from the span context
  // For now, this is a placeholder implementation
  // In a full implementation, baggage would be stored in the span context
  auto baggage_header = baggage_header_.get(trace_context);
  if (baggage_header.has_value()) {
    // Preserve existing baggage
    baggage_header_.setRefKey(trace_context, std::string(baggage_header.value()));
  }
}

std::vector<std::string> BaggagePropagator::fields() const {
  return {"baggage"};
}

std::string BaggagePropagator::name() const { return "baggage"; }

absl::flat_hash_map<std::string, std::string>
BaggagePropagator::parseBaggage(absl::string_view baggage_header) {
  absl::flat_hash_map<std::string, std::string> baggage_entries;

  // Parse comma-separated baggage entries
  for (const auto& entry : absl::StrSplit(baggage_header, ',')) {
    std::string trimmed_entry = std::string(absl::StripAsciiWhitespace(entry));
    if (trimmed_entry.empty()) {
      continue;
    }

    // Split key=value
    std::vector<std::string> kv = absl::StrSplit(trimmed_entry, absl::MaxSplits('=', 1));
    if (kv.size() == 2) {
      std::string key = urlDecode(absl::StripAsciiWhitespace(kv[0]));
      std::string value = urlDecode(absl::StripAsciiWhitespace(kv[1]));
      
      if (isValidBaggageKey(key) && isValidBaggageValue(value)) {
        baggage_entries[key] = value;
      }
    }
  }

  return baggage_entries;
}

std::string BaggagePropagator::formatBaggage(
    const absl::flat_hash_map<std::string, std::string>& baggage_entries) {
  std::vector<std::string> entries;
  
  for (const auto& [key, value] : baggage_entries) {
    if (isValidBaggageKey(key) && isValidBaggageValue(value)) {
      entries.push_back(absl::StrCat(urlEncode(key), "=", urlEncode(value)));
    }
  }

  return absl::StrJoin(entries, ",");
}

std::string BaggagePropagator::urlDecode(absl::string_view input) {
  // Simplified URL decode - in production would use proper URL decoding
  return std::string(input);
}

std::string BaggagePropagator::urlEncode(absl::string_view input) {
  // Simplified URL encode - in production would use proper URL encoding
  return std::string(input);
}

bool BaggagePropagator::isValidBaggageKey(absl::string_view key) {
  // Simplified validation - W3C spec has specific rules for baggage keys
  return !key.empty() && key.size() <= 256;
}

bool BaggagePropagator::isValidBaggageValue(absl::string_view value) {
  // Simplified validation - W3C spec has specific rules for baggage values
  return value.size() <= 4096;
}

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
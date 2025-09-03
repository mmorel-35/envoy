#include "source/extensions/propagators/w3c/trace_context.h"

#include <algorithm>
#include <cctype>

#include "absl/status/status.h"
#include "absl/strings/escaping.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3C {

// TraceParent implementation

TraceParent::TraceParent(absl::string_view version, absl::string_view trace_id,
                         absl::string_view parent_id, absl::string_view trace_flags)
    : version_(version), trace_id_(trace_id), parent_id_(parent_id), trace_flags_(trace_flags) {}

absl::StatusOr<TraceParent> TraceParent::parse(absl::string_view traceparent_value) {
  // Validate overall length
  if (traceparent_value.size() != Constants::kTraceparentHeaderSize) {
    return absl::InvalidArgumentError("Invalid traceparent header length");
  }

  // Split by hyphens - should result in exactly 4 parts
  std::vector<absl::string_view> parts = 
      absl::StrSplit(traceparent_value, '-', absl::SkipEmpty());
  if (parts.size() != 4) {
    return absl::InvalidArgumentError("Invalid traceparent format: must have 4 hyphen-separated fields");
  }

  absl::string_view version = parts[0];
  absl::string_view trace_id = parts[1];
  absl::string_view parent_id = parts[2];
  absl::string_view trace_flags = parts[3];

  // Validate field lengths
  if (version.size() != Constants::kVersionSize ||
      trace_id.size() != Constants::kTraceIdSize ||
      parent_id.size() != Constants::kParentIdSize ||
      trace_flags.size() != Constants::kTraceFlagsSize) {
    return absl::InvalidArgumentError("Invalid traceparent field sizes");
  }

  // Validate hex encoding
  if (!isValidHex(version) || !isValidHex(trace_id) || 
      !isValidHex(parent_id) || !isValidHex(trace_flags)) {
    return absl::InvalidArgumentError("Invalid traceparent hex encoding");
  }

  // Validate that trace-id and parent-id are not all zeros
  if (isAllZeros(trace_id)) {
    return absl::InvalidArgumentError("Invalid traceparent: trace-id cannot be all zeros");
  }
  if (isAllZeros(parent_id)) {
    return absl::InvalidArgumentError("Invalid traceparent: parent-id cannot be all zeros");
  }

  return TraceParent(version, trace_id, parent_id, trace_flags);
}

std::string TraceParent::toString() const {
  return absl::StrJoin({version_, trace_id_, parent_id_, trace_flags_}, "-");
}

bool TraceParent::isSampled() const {
  // Parse trace_flags as hex and check sampled bit
  std::string decoded = absl::HexStringToBytes(trace_flags_);
  if (decoded.empty()) {
    return false;
  }
  return (static_cast<uint8_t>(decoded[0]) & Constants::kSampledFlag) != 0;
}

void TraceParent::setSampled(bool sampled) {
  // Parse current flags
  std::string decoded = absl::HexStringToBytes(trace_flags_);
  if (decoded.empty()) {
    decoded = "\x00";
  }
  
  uint8_t flags = static_cast<uint8_t>(decoded[0]);
  if (sampled) {
    flags |= Constants::kSampledFlag;
  } else {
    flags &= ~Constants::kSampledFlag;
  }
  
  // Convert back to hex string
  trace_flags_ = absl::BytesToHexString(std::string(1, static_cast<char>(flags)));
  
  // Ensure uppercase hex (W3C spec uses lowercase, but be consistent)
  std::transform(trace_flags_.begin(), trace_flags_.end(), trace_flags_.begin(), ::tolower);
}

bool TraceParent::isValidHex(absl::string_view input) {
  return std::all_of(input.begin(), input.end(),
                     [](char c) { return std::isxdigit(c); });
}

bool TraceParent::isAllZeros(absl::string_view input) {
  return std::all_of(input.begin(), input.end(),
                     [](char c) { return c == '0'; });
}

// TraceState implementation

TraceState::TraceState(absl::string_view tracestate_value) {
  // If empty, nothing to do
  if (tracestate_value.empty()) {
    return;
  }

  // Parse comma-separated key=value pairs
  std::vector<absl::string_view> entries = absl::StrSplit(tracestate_value, ',');
  
  for (absl::string_view entry : entries) {
    entry = absl::StripAsciiWhitespace(entry);
    if (entry.empty()) {
      continue;
    }

    // Split on first '=' only
    std::vector<absl::string_view> kv = absl::StrSplit(entry, absl::MaxSplits('=', 1));
    if (kv.size() != 2) {
      continue; // Skip invalid entries
    }

    absl::string_view key = absl::StripAsciiWhitespace(kv[0]);
    absl::string_view value = absl::StripAsciiWhitespace(kv[1]);

    if (isValidKey(key) && isValidValue(value)) {
      entries_.emplace_back(key, value);
    }
  }
}

absl::StatusOr<TraceState> TraceState::parse(absl::string_view tracestate_value) {
  TraceState result(tracestate_value);
  return result;
}

std::string TraceState::toString() const {
  if (entries_.empty()) {
    return "";
  }

  std::vector<std::string> formatted_entries;
  formatted_entries.reserve(entries_.size());
  
  for (const auto& entry : entries_) {
    formatted_entries.push_back(absl::StrJoin({entry.first, entry.second}, "="));
  }
  
  return absl::StrJoin(formatted_entries, ",");
}

absl::optional<absl::string_view> TraceState::get(absl::string_view key) const {
  for (const auto& entry : entries_) {
    if (entry.first == key) {
      return entry.second;
    }
  }
  return absl::nullopt;
}

void TraceState::set(absl::string_view key, absl::string_view value) {
  if (!isValidKey(key) || !isValidValue(value)) {
    return; // Ignore invalid entries
  }

  // Remove existing entry with same key
  remove(key);
  
  // Add new entry at the beginning (as per W3C spec, most recent should be first)
  entries_.insert(entries_.begin(), std::make_pair(std::string(key), std::string(value)));
}

void TraceState::remove(absl::string_view key) {
  entries_.erase(
    std::remove_if(entries_.begin(), entries_.end(),
                   [&key](const std::pair<std::string, std::string>& entry) {
                     return entry.first == key;
                   }),
    entries_.end());
}

bool TraceState::isValidKey(absl::string_view key) {
  if (key.empty() || key.size() > 256) {
    return false;
  }
  
  // W3C spec: key must start with lowercase letter or digit, 
  // and contain only lowercase letters, digits, underscores, hyphens, asterisks, forward slashes
  for (size_t i = 0; i < key.size(); ++i) {
    char c = key[i];
    bool valid = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                 c == '_' || c == '-' || c == '*' || c == '/';
    if (!valid) {
      return false;
    }
  }
  
  return true;
}

bool TraceState::isValidValue(absl::string_view value) {
  if (value.empty() || value.size() > 256) {
    return false;
  }
  
  // W3C spec: value can contain any printable ASCII except comma, semicolon, and space
  for (char c : value) {
    if (c < 0x20 || c > 0x7E || c == ',' || c == ';' || c == ' ') {
      return false;
    }
  }
  
  return true;
}

// TraceContext implementation

TraceContext::TraceContext(TraceParent traceparent) : traceparent_(std::move(traceparent)) {}

TraceContext::TraceContext(TraceParent traceparent, TraceState tracestate)
    : traceparent_(std::move(traceparent)), tracestate_(std::move(tracestate)) {}

} // namespace W3C
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
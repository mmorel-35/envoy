#include "source/extensions/propagators/w3c/baggage/baggage_propagator.h"

#include <algorithm>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3c {
namespace Baggage {

namespace {

// Baggage validation helpers
bool isValidKey(absl::string_view key) {
  if (key.empty() || key.size() > Constants::kMaxKeyLength) {
    return false;
  }
  // Key should not contain special characters per W3C spec
  return std::all_of(key.begin(), key.end(), [](char c) {
    return std::isalnum(c) || c == '_' || c == '-' || c == '.' || c == '*';
  });
}

bool isValidValue(absl::string_view value) {
  if (value.size() > Constants::kMaxValueLength) {
    return false;
  }
  // Value should be URL-encoded safe characters
  return std::all_of(value.begin(), value.end(), [](char c) {
    return std::isalnum(c) || c == '_' || c == '-' || c == '.' || c == '*' || 
           c == '%' || c == '!' || c == '~' || c == '\'' || c == '(' || c == ')';
  });
}

// Parse a single baggage member: key=value;property1;property2
absl::StatusOr<BaggageMember> parseBaggageMember(absl::string_view member_str) {
  // Split on semicolon to separate key=value from properties
  std::vector<absl::string_view> parts = absl::StrSplit(member_str, ';');
  if (parts.empty()) {
    return absl::InvalidArgumentError("Empty baggage member");
  }

  // Parse key=value
  std::vector<absl::string_view> key_value = absl::StrSplit(parts[0], '=', absl::SkipEmpty());
  if (key_value.size() != 2) {
    return absl::InvalidArgumentError("Invalid baggage key=value format");
  }

  absl::string_view key = absl::StripAsciiWhitespace(key_value[0]);
  absl::string_view value = absl::StripAsciiWhitespace(key_value[1]);

  if (!isValidKey(key)) {
    return absl::InvalidArgumentError("Invalid baggage key");
  }
  if (!isValidValue(value)) {
    return absl::InvalidArgumentError("Invalid baggage value");
  }

  BaggageMember member;
  member.key = std::string(key);
  member.value = std::string(value);

  // Parse properties (if any)
  for (size_t i = 1; i < parts.size(); ++i) {
    absl::string_view property = absl::StripAsciiWhitespace(parts[i]);
    if (!property.empty()) {
      member.properties.push_back(std::string(property));
    }
  }

  return member;
}

} // namespace

BaggagePropagator::BaggagePropagator() = default;

absl::optional<std::string> BaggagePropagator::extractBaggage(
    const Tracing::TraceContext& trace_context) const {
  auto baggage_values = BaggageConstants::get().BAGGAGE.getAll(trace_context);
  if (baggage_values.empty()) {
    return absl::nullopt;
  }
  return absl::StrJoin(baggage_values, ",");
}

absl::StatusOr<BaggageMap> BaggagePropagator::parseBaggage(
    absl::string_view baggage_value) const {
  
  BaggageMap baggage_map;
  
  if (baggage_value.empty()) {
    return baggage_map;
  }

  // Check total size limit
  if (baggage_value.size() > Constants::kMaxBaggageSize) {
    return absl::InvalidArgumentError("Baggage exceeds maximum size limit");
  }

  // Split on comma to get individual members
  std::vector<absl::string_view> members = absl::StrSplit(baggage_value, ',');
  
  if (members.size() > Constants::kMaxBaggageMembers) {
    return absl::InvalidArgumentError("Baggage exceeds maximum member count");
  }

  for (absl::string_view member_str : members) {
    member_str = absl::StripAsciiWhitespace(member_str);
    if (member_str.empty()) {
      continue;
    }

    auto parsed_member = parseBaggageMember(member_str);
    if (!parsed_member.ok()) {
      return parsed_member.status();
    }

    baggage_map[parsed_member->key] = *parsed_member;
  }

  return baggage_map;
}

std::string BaggagePropagator::serializeBaggage(const BaggageMap& baggage_map) const {
  std::vector<std::string> members;
  
  for (const auto& [key, member] : baggage_map) {
    std::string member_str = absl::StrCat(key, "=", member.value);
    
    // Add properties if any
    for (const auto& property : member.properties) {
      absl::StrAppend(&member_str, ";", property);
    }
    
    members.push_back(member_str);
  }

  return absl::StrJoin(members, ",");
}

void BaggagePropagator::injectBaggage(
    Tracing::TraceContext& trace_context,
    const BaggageMap& baggage_map) const {
  
  if (baggage_map.empty()) {
    return;
  }

  std::string baggage_value = serializeBaggage(baggage_map);
  
  // Validate size before injection
  if (baggage_value.size() > Constants::kMaxBaggageSize) {
    // Silently truncate or skip oversized baggage rather than error
    return;
  }

  BaggageConstants::get().BAGGAGE.setRefKey(trace_context, baggage_value);
}

void BaggagePropagator::injectBaggage(
    Tracing::TraceContext& trace_context,
    absl::string_view baggage_value) const {
  
  if (!baggage_value.empty() && baggage_value.size() <= Constants::kMaxBaggageSize) {
    BaggageConstants::get().BAGGAGE.setRefKey(trace_context, baggage_value);
  }
}

void BaggagePropagator::removeBaggage(Tracing::TraceContext& trace_context) const {
  BaggageConstants::get().BAGGAGE.remove(trace_context);
}

bool BaggagePropagator::hasBaggage(const Tracing::TraceContext& trace_context) const {
  return BaggageConstants::get().BAGGAGE.get(trace_context).has_value();
}

absl::StatusOr<std::string> BaggagePropagator::getBaggageValue(
    const Tracing::TraceContext& trace_context,
    absl::string_view key) const {
  
  auto baggage_str = extractBaggage(trace_context);
  if (!baggage_str.has_value()) {
    return absl::NotFoundError("No baggage found");
  }

  auto baggage_map = parseBaggage(*baggage_str);
  if (!baggage_map.ok()) {
    return baggage_map.status();
  }

  auto it = baggage_map->find(std::string(key));
  if (it == baggage_map->end()) {
    return absl::NotFoundError(absl::StrCat("Baggage key '", key, "' not found"));
  }

  return it->second.value;
}

void BaggagePropagator::setBaggageValue(
    Tracing::TraceContext& trace_context,
    absl::string_view key,
    absl::string_view value) const {
  
  if (!isValidKey(key) || !isValidValue(value)) {
    return; // Silently ignore invalid keys/values
  }

  // Get existing baggage
  auto existing_baggage = extractBaggage(trace_context);
  BaggageMap baggage_map;
  
  if (existing_baggage.has_value()) {
    auto parsed = parseBaggage(*existing_baggage);
    if (parsed.ok()) {
      baggage_map = *parsed;
    }
  }

  // Add or update the key
  BaggageMember member;
  member.key = std::string(key);
  member.value = std::string(value);
  baggage_map[std::string(key)] = member;

  // Inject back
  injectBaggage(trace_context, baggage_map);
}

} // namespace Baggage
} // namespace W3c
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
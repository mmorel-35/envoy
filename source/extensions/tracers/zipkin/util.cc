#include "source/extensions/tracers/zipkin/util.h"

#include <chrono>
#include <random>
#include <regex>

#include "source/common/common/hex.h"
#include "source/common/common/utility.h"

#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace Zipkin {

uint64_t Util::generateRandom64(TimeSource& time_source) {
  uint64_t seed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      time_source.systemTime().time_since_epoch())
                      .count();
  std::mt19937_64 rand_64(seed);
  return rand_64();
}

Protobuf::Value Util::uint64Value(uint64_t value, absl::string_view name,
                                  Replacements& replacements) {
  const std::string string_value = std::to_string(value);
  replacements.push_back({absl::StrCat("\"", name, "\":\"", string_value, "\""),
                          absl::StrCat("\"", name, "\":", string_value)});
  return ValueUtil::stringValue(string_value);
}

std::string Util::uint64ToHexString(uint64_t value) {
  return Hex::uint64ToHex(value);
}

bool Util::parseTraceId(const std::string& trace_id_hex, uint64_t& trace_id_high,
                        uint64_t& trace_id_low) {
  if (trace_id_hex.length() == 16) {
    // 64-bit trace ID
    trace_id_high = 0;
    return absl::SimpleHexAtoi(trace_id_hex, &trace_id_low);
  } else if (trace_id_hex.length() == 32) {
    // 128-bit trace ID
    std::string high_part = trace_id_hex.substr(0, 16);
    std::string low_part = trace_id_hex.substr(16, 16);
    return absl::SimpleHexAtoi(high_part, &trace_id_high) &&
           absl::SimpleHexAtoi(low_part, &trace_id_low);
  }
  return false;
}

bool Util::parseSpanId(const std::string& span_id_hex, uint64_t& span_id) {
  if (span_id_hex.length() != 16) {
    return false;
  }
  return absl::SimpleHexAtoi(span_id_hex, &span_id);
}

uint64_t Util::generateRandom64() {
  uint64_t seed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count();
  std::mt19937_64 rand_64(seed);
  return rand_64();
}

} // namespace Zipkin
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

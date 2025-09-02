#include "source/extensions/propagators/common/trace_id_utils.h"

#include <chrono>
#include <random>

#include "absl/strings/numbers.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace Common {

bool TraceIdUtils::parseTraceId(const std::string& trace_id_hex, uint64_t& trace_id_high,
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

bool TraceIdUtils::parseSpanId(const std::string& span_id_hex, uint64_t& span_id) {
  if (span_id_hex.length() != 16) {
    return false;
  }
  return absl::SimpleHexAtoi(span_id_hex, &span_id);
}

uint64_t TraceIdUtils::generateRandom64(TimeSource& time_source) {
  uint64_t seed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      time_source.systemTime().time_since_epoch())
                      .count();
  std::mt19937_64 rand_64(seed);
  return rand_64();
}

uint64_t TraceIdUtils::generateRandom64() {
  uint64_t seed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count();
  std::mt19937_64 rand_64(seed);
  return rand_64();
}

} // namespace Common
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
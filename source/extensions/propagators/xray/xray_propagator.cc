#include "source/extensions/propagators/xray/xray_propagator.h"

#include "source/extensions/propagators/propagator_constants.h"
#include "source/common/common/hex.h"
#include "source/common/common/utility.h"

#include "absl/strings/str_split.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace XRay {

TraceHeader XRayPropagator::extract(const Tracing::TraceContext& trace_context) const {
  const auto& constants = PropagatorConstants::get();
  TraceHeader header;

  // Extract X-Amzn-Trace-Id header
  if (auto xray_header = constants.X_AMZN_TRACE_ID.get(trace_context)) {
    header = parseXAmznTraceId(*xray_header);
  }

  return header;
}

void XRayPropagator::inject(Tracing::TraceContext& trace_context, const TraceHeader& trace_header) const {
  const auto& constants = PropagatorConstants::get();

  // Inject X-Amzn-Trace-Id header
  if (trace_header.trace_id.has_value()) {
    auto xray_header = formatXAmznTraceId(trace_header);
    constants.X_AMZN_TRACE_ID.set(trace_context, xray_header);
  }
}

TraceHeader XRayPropagator::parseXAmznTraceId(absl::string_view xray_header) const {
  TraceHeader header;

  // Parse key-value pairs from X-Ray header
  auto pairs = parseKeyValuePairs(xray_header);

  // Extract Root (trace ID)
  auto root_it = pairs.find("Root");
  if (root_it != pairs.end() && isValidXRayTraceId(root_it->second)) {
    header.trace_id = xrayTraceIdToInternal(root_it->second);
  }

  // Extract Parent (span ID)
  auto parent_it = pairs.find("Parent");
  if (parent_it != pairs.end() && isValidXRaySpanId(parent_it->second)) {
    header.span_id = parent_it->second;
  }

  // Extract Sampled
  auto sampled_it = pairs.find("Sampled");
  if (sampled_it != pairs.end()) {
    if (sampled_it->second == "1") {
      header.sampled = true;
    } else if (sampled_it->second == "0") {
      header.sampled = false;
    }
  }

  return header;
}

std::string XRayPropagator::formatXAmznTraceId(const TraceHeader& trace_header) const {
  std::string result;

  // Add Root (trace ID)
  if (trace_header.trace_id.has_value()) {
    auto xray_trace_id = internalToXRayTraceId(*trace_header.trace_id);
    result = absl::StrCat("Root=", xray_trace_id);
  }

  // Add Parent (span ID)
  if (trace_header.span_id.has_value()) {
    if (!result.empty()) {
      result += ";";
    }
    result = absl::StrCat(result, "Parent=", *trace_header.span_id);
  }

  // Add Sampled
  if (trace_header.sampled.has_value()) {
    if (!result.empty()) {
      result += ";";
    }
    result = absl::StrCat(result, "Sampled=", *trace_header.sampled ? "1" : "0");
  }

  return result;
}

bool XRayPropagator::isValidXRayTraceId(absl::string_view trace_id) const {
  // X-Ray trace ID format: 1-{timestamp}-{unique-id}
  // Example: 1-5e1b4151-5ac2fbc4d7b3e8e4d1234567
  std::vector<absl::string_view> parts = absl::StrSplit(trace_id, '-');
  
  if (parts.size() != 3) {
    return false;
  }

  // Check version (must be "1")
  if (parts[0] != "1") {
    return false;
  }

  // Check timestamp (8 hex characters)
  if (parts[1].length() != 8 || !Hex::isValidHexString(parts[1])) {
    return false;
  }

  // Check unique ID (24 hex characters)
  if (parts[2].length() != 24 || !Hex::isValidHexString(parts[2])) {
    return false;
  }

  return true;
}

bool XRayPropagator::isValidXRaySpanId(absl::string_view span_id) const {
  // X-Ray span ID: 16 hex characters
  return span_id.length() == 16 && Hex::isValidHexString(span_id);
}

std::string XRayPropagator::xrayTraceIdToInternal(absl::string_view xray_trace_id) const {
  // Convert "1-5e1b4151-5ac2fbc4d7b3e8e4d1234567" to "5e1b41515ac2fbc4d7b3e8e4d1234567"
  std::vector<absl::string_view> parts = absl::StrSplit(xray_trace_id, '-');
  
  if (parts.size() != 3) {
    return std::string(xray_trace_id); // Return as-is if invalid format
  }

  // Combine timestamp and unique ID
  return absl::StrCat(parts[1], parts[2]);
}

std::string XRayPropagator::internalToXRayTraceId(absl::string_view internal_trace_id) const {
  // Convert "5e1b41515ac2fbc4d7b3e8e4d1234567" to "1-5e1b4151-5ac2fbc4d7b3e8e4d1234567"
  if (internal_trace_id.length() != 32) {
    return std::string(internal_trace_id); // Return as-is if invalid format
  }

  // Split into timestamp (first 8 chars) and unique ID (remaining 24 chars)
  auto timestamp = internal_trace_id.substr(0, 8);
  auto unique_id = internal_trace_id.substr(8);

  return absl::StrCat("1-", timestamp, "-", unique_id);
}

std::map<std::string, std::string> XRayPropagator::parseKeyValuePairs(absl::string_view header) const {
  std::map<std::string, std::string> pairs;

  // Split by semicolon
  std::vector<absl::string_view> segments = absl::StrSplit(header, ';');
  
  for (const auto& segment : segments) {
    // Split each segment by equals sign
    auto pos = segment.find('=');
    if (pos != absl::string_view::npos) {
      auto key = segment.substr(0, pos);
      auto value = segment.substr(pos + 1);
      
      // Trim whitespace and store
      pairs[std::string(absl::StripAsciiWhitespace(key))] = 
            std::string(absl::StripAsciiWhitespace(value));
    }
  }

  return pairs;
}

} // namespace XRay
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
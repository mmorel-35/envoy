#pragma once

#include "source/extensions/propagators/propagator_interface.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace XRay {

/**
 * AWS X-Ray trace propagator implementation.
 * 
 * Implements the AWS X-Ray trace context specification:
 * https://docs.aws.amazon.com/xray/latest/devguide/xray-concepts.html
 * 
 * Header:
 * - X-Amzn-Trace-Id: contains trace ID, parent ID, and sampling decision
 *   Format: Root={trace-id};Parent={parent-id};Sampled={0|1}
 *   Example: Root=1-5e1b4151-5ac2fbc4d7b3e8e4d1234567;Parent=3333333333333333;Sampled=1
 * 
 * Trace ID format:
 * - Version: 1
 * - Timestamp: Unix epoch time in seconds (8 hex digits)
 * - Unique ID: 96-bit unique identifier (24 hex digits)
 * - Total: 1-{timestamp}-{unique-id}
 */
class XRayPropagator : public Propagator {
public:
  XRayPropagator() = default;

  // Propagator interface
  TraceHeader extract(const Tracing::TraceContext& trace_context) const override;
  void inject(Tracing::TraceContext& trace_context, const TraceHeader& trace_header) const override;
  absl::string_view name() const override { return "xray"; }

private:
  /**
   * Parse X-Amzn-Trace-Id header.
   * Format: Root={trace-id};Parent={parent-id};Sampled={0|1}
   */
  TraceHeader parseXAmznTraceId(absl::string_view xray_header) const;

  /**
   * Generate X-Amzn-Trace-Id header.
   * Format: Root={trace-id};Parent={parent-id};Sampled={0|1}
   */
  std::string formatXAmznTraceId(const TraceHeader& trace_header) const;

  /**
   * Validate X-Ray trace ID format.
   * Format: 1-{timestamp}-{unique-id}
   */
  bool isValidXRayTraceId(absl::string_view trace_id) const;

  /**
   * Validate X-Ray span ID format (16 hex characters).
   */
  bool isValidXRaySpanId(absl::string_view span_id) const;

  /**
   * Convert X-Ray trace ID to internal format (remove version and timestamp prefix).
   * Converts: "1-5e1b4151-5ac2fbc4d7b3e8e4d1234567" -> "5e1b41515ac2fbc4d7b3e8e4d1234567"
   */
  std::string xrayTraceIdToInternal(absl::string_view xray_trace_id) const;

  /**
   * Convert internal trace ID to X-Ray format (add version and timestamp).
   * Converts: "5e1b41515ac2fbc4d7b3e8e4d1234567" -> "1-5e1b4151-5ac2fbc4d7b3e8e4d1234567"
   */
  std::string internalToXRayTraceId(absl::string_view internal_trace_id) const;

  /**
   * Extract key-value pairs from X-Ray header.
   */
  std::map<std::string, std::string> parseKeyValuePairs(absl::string_view header) const;
};

} // namespace XRay
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
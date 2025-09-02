#pragma once

#include "envoy/common/exception.h"
#include "envoy/tracing/tracer.h"

#include "source/common/http/header_map_impl.h"
#include "source/extensions/tracers/opentelemetry/span_context_extractor.h"
#include "source/extensions/tracers/opentelemetry/propagators/propagator.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace Zipkin {

class SpanContext;

struct ExtractorException : public EnvoyException {
  ExtractorException(const std::string& what) : EnvoyException(what) {}
};

/**
 * This class is used to SpanContext extracted from the Http header.
 * Supports B3 propagation format natively and W3C Trace Context as fallback
 * when w3c_fallback_enabled is true.
 */
class SpanContextExtractor {
public:
  /**
   * Constructor for B3-only extraction.
   * @param trace_context HTTP headers to extract from
   * @param w3c_fallback_enabled Whether to enable W3C Trace Context fallback
   */
  SpanContextExtractor(Tracing::TraceContext& trace_context, bool w3c_fallback_enabled = false);
  
  /**
   * Constructor with configured W3C propagator names for OpenTelemetry specification compliance.
   * This allows respecting OTEL_PROPAGATORS environment variable and custom propagator configuration.
   * @param trace_context HTTP headers to extract from
   * @param w3c_fallback_enabled Whether to enable W3C Trace Context fallback
   * @param w3c_propagator_names List of propagator names to use for W3C fallback (e.g., "tracecontext", "b3")
   */
  SpanContextExtractor(Tracing::TraceContext& trace_context, bool w3c_fallback_enabled,
                       const std::vector<std::string>& w3c_propagator_names);
  ~SpanContextExtractor();
  absl::optional<bool> extractSampled();
  std::pair<SpanContext, bool> extractSpanContext(bool is_sampled);

private:
  /*
   * Use to SpanContext extracted from B3 single format Http header
   * b3: {x-b3-traceid}-{x-b3-spanid}-{if x-b3-flags 'd' else x-b3-sampled}-{x-b3-parentspanid}
   * See: "https://github.com/openzipkin/b3-propagation
   */
  std::pair<SpanContext, bool> extractSpanContextFromB3SingleFormat(bool is_sampled);

  /*
   * Convert W3C span context to Zipkin span context format
   */
  std::pair<SpanContext, bool>
  convertW3CToZipkin(const Extensions::Tracers::OpenTelemetry::SpanContext& w3c_context,
                     bool fallback_sampled);

  bool tryExtractSampledFromB3SingleFormat();
  const Tracing::TraceContext& trace_context_;
  bool w3c_fallback_enabled_;
  std::vector<std::string> w3c_propagator_names_;
};

} // namespace Zipkin
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

#pragma once

#include "envoy/common/exception.h"
#include "envoy/tracing/tracer.h"

#include "source/common/http/header_map_impl.h"
#include "source/extensions/tracers/opentelemetry/span_context_extractor.h"
#include "source/extensions/propagators/opentelemetry/propagator.h"

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
   * 
   * For full OpenTelemetry specification compliance, the caller should:
   * 1. Read OTEL_PROPAGATORS environment variable using Api::Api interface
   * 2. Parse the propagator names using PropagatorFactory::parseOtelPropagatorsEnv()
   * 3. Pass the parsed names to this constructor
   * 
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
   * Convert W3C span context to Zipkin span context format
   */
  std::pair<SpanContext, bool>
  convertW3CToZipkin(const Extensions::Tracers::OpenTelemetry::SpanContext& w3c_context,
                     bool fallback_sampled);

  const Tracing::TraceContext& trace_context_;
  bool w3c_fallback_enabled_;
  std::vector<std::string> w3c_propagator_names_;
};

} // namespace Zipkin
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

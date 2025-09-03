#pragma once

#include "envoy/common/exception.h"
#include "envoy/tracing/tracer.h"

#include "source/common/http/header_map_impl.h"
#include "source/extensions/propagators/b3/propagator.h"
#include "source/extensions/propagators/w3c/propagator.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace Zipkin {

class SpanContext;

struct ExtractorException : public EnvoyException {
  ExtractorException(const std::string& what) : EnvoyException(what) {}
};

/**
 * This class is used to extract SpanContext from HTTP headers.
 * Refactored to use the new B3 and W3C propagators for better
 * specification compliance and reduced code duplication.
 */
class SpanContextExtractor {
public:
  SpanContextExtractor(Tracing::TraceContext& trace_context, bool w3c_fallback_enabled = false);
  ~SpanContextExtractor();
  absl::optional<bool> extractSampled();
  std::pair<SpanContext, bool> extractSpanContext(bool is_sampled);

private:
  /*
   * Convert B3 trace context to Zipkin span context format
   */
  std::pair<SpanContext, bool> convertB3ToZipkin(
      const Extensions::Propagators::B3::TraceContext& b3_context, bool fallback_sampled);

  /*
   * Convert W3C trace context to Zipkin span context format
   */
  std::pair<SpanContext, bool> convertW3CToZipkin(
      const Extensions::Propagators::W3C::TraceContext& w3c_context, bool fallback_sampled);

  const Tracing::TraceContext& trace_context_;
  bool w3c_fallback_enabled_;
};

} // namespace Zipkin
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

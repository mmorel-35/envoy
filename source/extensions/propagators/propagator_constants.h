#pragma once

#include <string>

#include "envoy/http/header_map.h"

#include "source/common/singleton/const_singleton.h"
#include "source/common/tracing/trace_context_impl.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {

namespace {

// Common constants
constexpr char SAMPLED[] = "1";
constexpr char NOT_SAMPLED[] = "0";

// B3 specific constants
constexpr char B3_TRUE[] = "1";
constexpr char B3_FALSE[] = "0";
constexpr char B3_DEBUG[] = "d";

// X-Ray specific constants
constexpr char XRAY_ROOT_PREFIX[] = "Root=";
constexpr char XRAY_PARENT_PREFIX[] = "Parent=";
constexpr char XRAY_SAMPLED_PREFIX[] = "Sampled=";

} // namespace

/**
 * Contains all the propagator-related constants to avoid duplication across
 * different trace propagator implementations.
 */
class PropagatorConstantValues {
public:
  // B3 multi-header format headers (https://github.com/openzipkin/b3-propagation)
  const Tracing::TraceContextHandler X_B3_TRACE_ID{"x-b3-traceid"};
  const Tracing::TraceContextHandler X_B3_SPAN_ID{"x-b3-spanid"};
  const Tracing::TraceContextHandler X_B3_PARENT_SPAN_ID{"x-b3-parentspanid"};
  const Tracing::TraceContextHandler X_B3_SAMPLED{"x-b3-sampled"};
  const Tracing::TraceContextHandler X_B3_FLAGS{"x-b3-flags"};

  // B3 single header format
  const Tracing::TraceContextHandler B3{"b3"};

  // W3C Trace Context headers (https://www.w3.org/TR/trace-context/)
  const Tracing::TraceContextHandler TRACE_PARENT{"traceparent"};
  const Tracing::TraceContextHandler TRACE_STATE{"tracestate"};

  // AWS X-Ray headers (https://docs.aws.amazon.com/xray/latest/devguide/xray-concepts.html)
  const Tracing::TraceContextHandler X_AMZN_TRACE_ID{"x-amzn-trace-id"};

  // Jaeger headers (for future extension)
  const Tracing::TraceContextHandler UBER_TRACE_ID{"uber-trace-id"};

  // OpenTelemetry headers (for future extension) 
  const Tracing::TraceContextHandler BAGGAGE{"baggage"};
};

using PropagatorConstants = ConstSingleton<PropagatorConstantValues>;

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
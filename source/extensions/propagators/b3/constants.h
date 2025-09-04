#pragma once

#include <string>

#include "envoy/http/header_map.h"

#include "source/common/singleton/const_singleton.h"
#include "source/common/tracing/trace_context_impl.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace B3 {

/**
 * B3 Trace Propagation constants for header names as defined in:
 * https://github.com/openzipkin/b3-propagation
 */
class ConstantValues {
public:
  // B3 multi-header format headers
  const Tracing::TraceContextHandler X_B3_TRACE_ID{"x-b3-traceid"};
  const Tracing::TraceContextHandler X_B3_SPAN_ID{"x-b3-spanid"};
  const Tracing::TraceContextHandler X_B3_PARENT_SPAN_ID{"x-b3-parentspanid"};
  const Tracing::TraceContextHandler X_B3_SAMPLED{"x-b3-sampled"};
  const Tracing::TraceContextHandler X_B3_FLAGS{"x-b3-flags"};

  // B3 single header format
  const Tracing::TraceContextHandler B3{"b3"};
};

using Constants = ConstSingleton<ConstantValues>;

} // namespace B3
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
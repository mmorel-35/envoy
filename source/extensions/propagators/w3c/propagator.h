#pragma once

#include <string>

#include "envoy/http/header_map.h"

#include "source/common/singleton/const_singleton.h"
#include "source/common/tracing/trace_context_impl.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3c {

/**
 * W3C Trace Context constants for header names as defined in:
 * https://www.w3.org/TR/trace-context/
 */
class W3cConstantValues {
public:
  // W3C Trace Context headers
  const Tracing::TraceContextHandler TRACE_PARENT{"traceparent"};
  const Tracing::TraceContextHandler TRACE_STATE{"tracestate"};
};

using W3cConstants = ConstSingleton<W3cConstantValues>;

} // namespace W3c
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
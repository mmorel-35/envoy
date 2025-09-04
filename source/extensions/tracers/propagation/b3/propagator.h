#pragma once

#include <string>

#include "envoy/http/header_map.h"

#include "source/common/singleton/const_singleton.h"
#include "source/common/tracing/trace_context_impl.h"
#include "source/extensions/tracers/propagation/b3/trace_context.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace Propagation {
namespace B3 {

/**
 * B3 Trace Propagation constants for header names as defined in:
 * https://github.com/openzipkin/b3-propagation
 */
class B3ConstantValues {
public:
  // B3 multi-header format headers
  const Tracing::TraceContextHandler X_B3_TRACE_ID{std::string(Constants::kTraceIdHeader)};
  const Tracing::TraceContextHandler X_B3_SPAN_ID{std::string(Constants::kSpanIdHeader)};
  const Tracing::TraceContextHandler X_B3_PARENT_SPAN_ID{std::string(Constants::kParentSpanIdHeader)};
  const Tracing::TraceContextHandler X_B3_SAMPLED{std::string(Constants::kSampledHeader)};
  const Tracing::TraceContextHandler X_B3_FLAGS{std::string(Constants::kFlagsHeader)};

  // B3 single header format
  const Tracing::TraceContextHandler B3{std::string(Constants::kB3Header)};
};

using B3Constants = ConstSingleton<B3ConstantValues>;

} // namespace B3
} // namespace Propagation
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
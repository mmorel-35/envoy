#pragma once

#include <string>

#include "envoy/http/header_map.h"

#include "source/common/singleton/const_singleton.h"
#include "source/common/tracing/trace_context_impl.h"
#include "source/extensions/propagators/w3c/trace_context.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3c {

/**
 * W3C Trace Context constants for header names as defined in:
 * https://www.w3.org/TR/trace-context/
 */
class W3CConstantValues {
public:
  const Tracing::TraceContextHandler TRACE_PARENT{std::string(Constants::kTraceparentHeader)};
  const Tracing::TraceContextHandler TRACE_STATE{std::string(Constants::kTracestateHeader)};
  const Tracing::TraceContextHandler BAGGAGE{std::string(Constants::kBaggageHeader)};
};

using W3cConstants = ConstSingleton<W3CConstantValues>;

} // namespace W3c
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

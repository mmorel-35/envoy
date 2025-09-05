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

namespace TraceContext {
/**
 * W3C Trace Context constants for header names as defined in:
 * https://www.w3.org/TR/trace-context/
 */
class TraceContextConstantValues {
public:
  const Tracing::TraceContextHandler TRACE_PARENT{std::string(Constants::kTraceparentHeader)};
  const Tracing::TraceContextHandler TRACE_STATE{std::string(Constants::kTracestateHeader)};
};

using TraceContextConstants = ConstSingleton<TraceContextConstantValues>;
} // namespace TraceContext

namespace Baggage {
/**
 * W3C Baggage constants for header names as defined in:
 * https://www.w3.org/TR/baggage/
 */
class BaggageConstantValues {
public:
  const Tracing::TraceContextHandler BAGGAGE{std::string(Constants::kBaggageHeader)};
};

using BaggageConstants = ConstSingleton<BaggageConstantValues>;
} // namespace Baggage

/**
 * Legacy W3C Trace Context constants for backward compatibility.
 * New code should use TraceContext:: or Baggage:: namespaces directly.
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

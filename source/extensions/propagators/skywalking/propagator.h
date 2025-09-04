#pragma once

#include <string>

#include "envoy/http/header_map.h"

#include "source/common/singleton/const_singleton.h"
#include "source/common/tracing/trace_context_impl.h"
#include "source/extensions/propagators/skywalking/trace_context.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace SkyWalking {

/**
 * SkyWalking trace propagation constants for header names.
 */
class SkyWalkingConstantValues {
public:
  // SkyWalking trace header
  const Tracing::TraceContextHandler SW8{std::string(Constants::kSw8Header)};
};

using SkyWalkingConstants = ConstSingleton<SkyWalkingConstantValues>;

} // namespace SkyWalking
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy

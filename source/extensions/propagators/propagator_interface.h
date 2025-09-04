#pragma once

#include <string>

#include "envoy/common/pure.h"
#include "envoy/tracing/trace_context.h"

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {

/**
 * Represents extracted trace context information from headers.
 */
struct TraceHeader {
  absl::optional<std::string> trace_id;
  absl::optional<std::string> span_id;
  absl::optional<std::string> parent_span_id;
  absl::optional<bool> sampled;
  absl::optional<std::string> trace_state;
};

/**
 * Base interface for all trace propagators.
 * Trace propagators are responsible for extracting and injecting
 * trace context information from/to trace contexts (e.g., HTTP headers).
 */
class Propagator {
public:
  virtual ~Propagator() = default;

  /**
   * Extract trace context information from the given trace context.
   * @param trace_context the trace context to extract from
   * @return TraceHeader containing extracted trace information
   */
  virtual TraceHeader extract(const Tracing::TraceContext& trace_context) const PURE;

  /**
   * Inject trace context information into the given trace context.
   * @param trace_context the trace context to inject into
   * @param trace_header the trace information to inject
   */
  virtual void inject(Tracing::TraceContext& trace_context, const TraceHeader& trace_header) const PURE;

  /**
   * Get the name of the propagator (e.g., "b3", "w3c", "xray").
   * @return the name of the propagator
   */
  virtual absl::string_view name() const PURE;
};

using PropagatorPtr = std::unique_ptr<Propagator>;

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
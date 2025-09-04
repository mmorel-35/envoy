#pragma once

/**
 * Example usage of the propagators skeleton.
 * This demonstrates how the different propagators can be used
 * in a real Envoy scenario.
 */

#include "source/extensions/propagators/b3/b3_propagator.h"
#include "source/extensions/propagators/w3c/w3c_propagator.h"
#include "source/extensions/propagators/xray/xray_propagator.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {

/**
 * Example of a propagator manager that can handle multiple formats.
 * This demonstrates the power of the unified interface.
 */
class PropagatorManager {
public:
  PropagatorManager() {
    // Register all available propagators
    propagators_["b3"] = std::make_unique<B3::B3Propagator>();
    propagators_["w3c"] = std::make_unique<W3C::W3CPropagator>();
    propagators_["xray"] = std::make_unique<XRay::XRayPropagator>();
  }

  /**
   * Try to extract trace context using all available propagators.
   * Returns the first successful extraction.
   */
  absl::optional<TraceHeader> extractAny(const Tracing::TraceContext& trace_context) const {
    for (const auto& [name, propagator] : propagators_) {
      auto header = propagator->extract(trace_context);
      if (header.trace_id.has_value()) {
        return header;
      }
    }
    return absl::nullopt;
  }

  /**
   * Inject trace context using a specific propagator.
   */
  bool inject(const std::string& propagator_name, 
              Tracing::TraceContext& trace_context, 
              const TraceHeader& trace_header) const {
    auto it = propagators_.find(propagator_name);
    if (it != propagators_.end()) {
      it->second->inject(trace_context, trace_header);
      return true;
    }
    return false;
  }

  /**
   * Convert between propagation formats.
   * Extract from one format and inject into another.
   */
  bool convert(const std::string& from_format,
               const std::string& to_format,
               Tracing::TraceContext& trace_context) const {
    auto from_it = propagators_.find(from_format);
    auto to_it = propagators_.find(to_format);
    
    if (from_it == propagators_.end() || to_it == propagators_.end()) {
      return false;
    }

    // Extract from source format
    auto trace_header = from_it->second->extract(trace_context);
    if (!trace_header.trace_id.has_value()) {
      return false;
    }

    // Inject into target format
    to_it->second->inject(trace_context, trace_header);
    return true;
  }

private:
  std::map<std::string, PropagatorPtr> propagators_;
};

/**
 * Example of format-specific usage scenarios.
 */
namespace Examples {

// Example 1: B3 multi-header extraction and single-header injection
void b3FormatConversion(Tracing::TraceContext& trace_context) {
  B3::B3Propagator b3_propagator;
  
  // Extract from multi-header format
  auto trace_header = b3_propagator.extract(trace_context);
  
  if (trace_header.trace_id.has_value()) {
    // Could inject as single header or keep as multi-header
    b3_propagator.inject(trace_context, trace_header);
  }
}

// Example 2: Convert from B3 to W3C format
void convertB3ToW3C(Tracing::TraceContext& trace_context) {
  B3::B3Propagator b3_propagator;
  W3C::W3CPropagator w3c_propagator;
  
  // Extract B3 trace context
  auto trace_header = b3_propagator.extract(trace_context);
  
  if (trace_header.trace_id.has_value()) {
    // Inject as W3C format
    w3c_propagator.inject(trace_context, trace_header);
  }
}

// Example 3: X-Ray trace context handling
void handleXRayTracing(Tracing::TraceContext& trace_context) {
  XRay::XRayPropagator xray_propagator;
  
  // Extract X-Ray context
  auto trace_header = xray_propagator.extract(trace_context);
  
  if (trace_header.trace_id.has_value()) {
    // X-Ray trace IDs have special format: 1-{timestamp}-{unique-id}
    // The propagator handles the conversion automatically
    
    // Can inject the same context or propagate to other systems
    xray_propagator.inject(trace_context, trace_header);
  }
}

// Example 4: Multi-format trace propagation chain
void propagationChain(Tracing::TraceContext& incoming_context,
                     Tracing::TraceContext& outgoing_context) {
  PropagatorManager manager;
  
  // Try to extract from any supported format
  auto trace_header = manager.extractAny(incoming_context);
  
  if (trace_header.has_value()) {
    // Propagate using W3C format (most widely supported)
    manager.inject("w3c", outgoing_context, *trace_header);
  }
}

} // namespace Examples

} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
#pragma once

// Example integration showing how existing tracers can use the W3C propagator

#include "source/extensions/propagators/w3c/propagator.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {

/**
 * Example of how an existing tracer can integrate with the W3C propagator
 * to eliminate code duplication while maintaining backward compatibility.
 */
class ExampleTracerUsingW3C {
public:
  using W3C = Extensions::Propagators::W3C;

  // Example: Check if W3C headers are present (replaces custom propagationHeaderPresent())
  bool w3cHeadersPresent(const Tracing::TraceContext& trace_context) {
    return W3C::Propagator::isPresent(trace_context);
  }

  // Example: Extract span context using W3C propagator (replaces custom extractSpanContext())
  absl::StatusOr<YourSpanContextType>
  extractSpanContext(const Tracing::TraceContext& trace_context) {
    // Use the helper for backward compatibility
    auto extracted = W3C::TracingHelper::extractForTracer(trace_context);
    if (!extracted.has_value()) {
      return absl::InvalidArgumentError("No W3C trace context found");
    }

    const auto& w3c_values = extracted.value();

    // Convert to your tracer's span context format
    return YourSpanContextType{.version = w3c_values.version,
                               .trace_id = w3c_values.trace_id,
                               .span_id = w3c_values.span_id,
                               .sampled = w3c_values.sampled,
                               .tracestate = w3c_values.tracestate};
  }

  // Example: Inject headers using W3C propagator
  void injectHeaders(const YourSpanContextType& span_context,
                     Tracing::TraceContext& trace_context) {
    // Create W3C context from your span context
    auto traceparent =
        W3C::TraceParent::parse(absl::StrJoin({span_context.version, span_context.trace_id,
                                               span_context.span_id, span_context.trace_flags},
                                              "-"));

    if (!traceparent.ok()) {
      return; // Handle error
    }

    W3C::TraceState tracestate;
    if (!span_context.tracestate.empty()) {
      auto parsed_tracestate = W3C::TraceState::parse(span_context.tracestate);
      if (parsed_tracestate.ok()) {
        tracestate = std::move(parsed_tracestate.value());
      }
    }

    // Include baggage if present
    W3C::Baggage baggage;
    if (!span_context.baggage.empty()) {
      auto parsed_baggage = W3C::Baggage::parse(span_context.baggage);
      if (parsed_baggage.ok()) {
        baggage = std::move(parsed_baggage.value());
      }
    }

    W3C::TraceContext w3c_context(std::move(traceparent.value()), std::move(tracestate),
                                  std::move(baggage));
    W3C::Propagator::inject(w3c_context, trace_context);
  }

  // Example: Implement standard Span baggage interface using W3C propagator
  std::string getBaggage(const Tracing::TraceContext& trace_context, absl::string_view key) {
    return W3C::BaggageHelper::getBaggageValue(trace_context, key);
  }

  bool setBaggage(Tracing::TraceContext& trace_context, absl::string_view key,
                  absl::string_view value) {
    return W3C::BaggageHelper::setBaggageValue(trace_context, key, value);
  }

private:
  // Your existing span context type
  struct YourSpanContextType {
    std::string version;
    std::string trace_id;
    std::string span_id;
    std::string trace_flags;
    bool sampled;
    std::string tracestate;
    std::string baggage; // Added baggage support
  };
};

/**
 * Migration guide for existing tracers:
 *
 * BEFORE (duplicated across tracers):
 * ```cpp
 * constexpr int kTraceparentHeaderSize = 55;
 * constexpr int kVersionHexSize = 2;
 * // ... more constants duplicated
 *
 * bool isValidHex(const absl::string_view& input) {
 *   // ... duplicated validation logic
 * }
 *
 * class SpanContextExtractor {
 *   absl::StatusOr<SpanContext> extractSpanContext() {
 *     // ... duplicated parsing logic
 *   }
 * };
 * ```
 *
 * AFTER (using W3C propagator):
 * ```cpp
 * using W3C = Extensions::Propagators::W3C;
 *
 * class SpanContextExtractor {
 *   absl::StatusOr<SpanContext> extractSpanContext() {
 *     auto extracted = W3C::TracingHelper::extractForTracer(trace_context_);
 *     if (!extracted.has_value()) {
 *       return absl::InvalidArgumentError("No W3C trace context found");
 *     }
 *
 *     // Convert to your format
 *     return convertFromW3C(extracted.value());
 *   }
 * };
 * ```
 *
 * Benefits:
 * - Eliminates ~200 lines of duplicated code per tracer
 * - Ensures W3C specification compliance
 * - Centralized validation and parsing logic
 * - Easier to maintain and extend
 * - Full W3C Baggage support for span baggage interface
 *
 * W3C BAGGAGE INTEGRATION:
 * ```cpp
 * // Traditional span baggage interface using W3C propagator
 * class MySpan : public Tracing::Span {
 * public:
 *   std::string getBaggage(absl::string_view key) override {
 *     return W3C::BaggageHelper::getBaggageValue(trace_context_, key);
 *   }
 *
 *   void setBaggage(absl::string_view key, absl::string_view value) override {
 *     W3C::BaggageHelper::setBaggageValue(trace_context_, key, value);
 *   }
 * };
 *
 * // Advanced baggage manipulation
 * auto baggage = W3C::Propagator::extractBaggage(trace_context);
 * if (baggage.ok()) {
 *   baggage.value().set("userId", "alice");
 *   baggage.value().set("sessionId", "xyz123");
 *   W3C::Propagator::injectBaggage(baggage.value(), trace_context);
 * }
 * ```
 */

} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
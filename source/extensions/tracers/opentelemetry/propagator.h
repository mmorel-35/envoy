#pragma once

#include "envoy/tracing/tracer.h"

#include "source/common/common/statusor.h"
#include "source/extensions/tracers/opentelemetry/span_context.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

/**
 * Abstract interface for trace context propagation.
 * Each propagator handles a specific format (W3C, B3, etc.)
 */
class TextMapPropagator {
public:
  virtual ~TextMapPropagator() = default;

  /**
   * Extract span context from trace context headers.
   * @param trace_context The HTTP headers to extract from.
   * @return SpanContext if extraction succeeds, error status otherwise.
   */
  virtual absl::StatusOr<SpanContext> extract(const Tracing::TraceContext& trace_context) = 0;

  /**
   * Inject span context into trace context headers.
   * @param span_context The span context to inject.
   * @param trace_context The HTTP headers to inject into.
   */
  virtual void inject(const SpanContext& span_context, Tracing::TraceContext& trace_context) = 0;

  /**
   * @return The list of header names this propagator reads/writes.
   */
  virtual std::vector<std::string> fields() const = 0;

  /**
   * @return The name of this propagator (for logging/debugging).
   */
  virtual std::string name() const = 0;
};

using TextMapPropagatorPtr = std::unique_ptr<TextMapPropagator>;

/**
 * Manages multiple propagators and coordinates extraction/injection.
 */
class CompositePropagator {
public:
  explicit CompositePropagator(std::vector<TextMapPropagatorPtr> propagators);

  /**
   * Try to extract span context using configured propagators in order.
   * @param trace_context The HTTP headers to extract from.
   * @return SpanContext from first successful propagator, error if none succeed.
   */
  absl::StatusOr<SpanContext> extract(const Tracing::TraceContext& trace_context);

  /**
   * Inject span context using all configured propagators.
   * @param span_context The span context to inject.
   * @param trace_context The HTTP headers to inject into.
   */
  void inject(const SpanContext& span_context, Tracing::TraceContext& trace_context);

  /**
   * Check if any propagation headers are present.
   * @param trace_context The HTTP headers to check.
   * @return True if any propagator detects its headers.
   */
  bool propagationHeaderPresent(const Tracing::TraceContext& trace_context);

private:
  std::vector<TextMapPropagatorPtr> propagators_;
};

using CompositePropagatorPtr = std::unique_ptr<CompositePropagator>;

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
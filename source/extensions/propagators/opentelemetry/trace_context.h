#pragma once

#include "source/extensions/propagators/w3c/trace_context.h"
#include "source/extensions/propagators/b3/trace_context.h"

#include "absl/types/variant.h"
#include "absl/types/optional.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace OpenTelemetry {

/**
 * OpenTelemetry Composite Trace Context represents a unified view of distributed
 * tracing context that can contain either W3C or B3 trace information.
 * 
 * This implements the OpenTelemetry specification for composite propagators:
 * https://opentelemetry.io/docs/specs/otel/context/api-propagators
 * 
 * The composite context allows OpenTelemetry to work with multiple trace formats
 * while providing a single, unified interface for extraction and injection.
 */

/**
 * Enum representing the format type of the trace context.
 */
enum class TraceFormat {
  W3C,    // W3C Trace Context format
  B3,     // B3 Propagation format  
  NONE    // No valid trace context found
};

/**
 * CompositeTraceContext holds either W3C or B3 trace context information.
 * This allows the OpenTelemetry propagator to work with multiple formats
 * while providing a unified interface.
 */
class CompositeTraceContext {
public:
  /**
   * Default constructor creates empty context.
   */
  CompositeTraceContext() : format_(TraceFormat::NONE) {}

  /**
   * Constructor from W3C trace context.
   * @param w3c_context W3C trace context
   */
  explicit CompositeTraceContext(const W3C::TraceContext& w3c_context);

  /**
   * Constructor from B3 trace context.
   * @param b3_context B3 trace context
   */
  explicit CompositeTraceContext(const B3::TraceContext& b3_context);

  /**
   * Get the format type of this context.
   * @return TraceFormat enum indicating the underlying format
   */
  TraceFormat format() const { return format_; }

  /**
   * Check if this context contains valid trace information.
   * @return true if context contains W3C or B3 data
   */
  bool isValid() const { return format_ != TraceFormat::NONE; }

  /**
   * Get W3C trace context (if available).
   * @return W3C TraceContext or nullopt if not W3C format
   */
  absl::optional<W3C::TraceContext> getW3CContext() const;

  /**
   * Get B3 trace context (if available).
   * @return B3 TraceContext or nullopt if not B3 format
   */
  absl::optional<B3::TraceContext> getB3Context() const;

  /**
   * Extract trace ID as string regardless of underlying format.
   * @return trace ID in hex format (32 chars for W3C, 16 or 32 for B3)
   */
  std::string getTraceId() const;

  /**
   * Extract span ID as string regardless of underlying format.
   * @return span ID in hex format (16 chars)
   */
  std::string getSpanId() const;

  /**
   * Extract parent span ID as string regardless of underlying format.
   * @return parent span ID in hex format, empty if no parent
   */
  std::string getParentSpanId() const;

  /**
   * Check if trace is sampled regardless of underlying format.
   * @return true if trace should be sampled
   */
  bool isSampled() const;

  /**
   * Get trace state information (W3C only).
   * @return trace state string, empty for B3 format
   */
  std::string getTraceState() const;

  /**
   * Create a child span context from this context.
   * @param new_span_id the new span ID (16 hex characters)
   * @return new CompositeTraceContext with updated parent-id
   */
  absl::StatusOr<CompositeTraceContext> createChild(absl::string_view new_span_id) const;

  /**
   * Convert this context to a specific format.
   * @param target_format the desired format
   * @return CompositeTraceContext in target format, or error if conversion fails
   */
  absl::StatusOr<CompositeTraceContext> convertTo(TraceFormat target_format) const;

private:
  TraceFormat format_;
  absl::variant<absl::monostate, W3C::TraceContext, B3::TraceContext> context_;
};

/**
 * CompositeBaggage represents baggage information that can be extracted
 * from various propagation formats.
 * 
 * Currently, only W3C format supports baggage, but this provides a unified
 * interface for future extensions.
 */
class CompositeBaggage {
public:
  /**
   * Default constructor creates empty baggage.
   */
  CompositeBaggage() = default;

  /**
   * Constructor from W3C baggage.
   * @param w3c_baggage W3C baggage
   */
  explicit CompositeBaggage(const W3C::Baggage& w3c_baggage);

  /**
   * Check if baggage contains any entries.
   * @return true if baggage has entries
   */
  bool isEmpty() const;

  /**
   * Get baggage value by key.
   * @param key the baggage key
   * @return baggage value if found, empty string otherwise
   */
  std::string getValue(absl::string_view key) const;

  /**
   * Set baggage value.
   * @param key the baggage key
   * @param value the baggage value
   * @return true if successfully set, false if size limits exceeded
   */
  bool setValue(absl::string_view key, absl::string_view value);

  /**
   * Get all baggage as a map.
   * @return map of all baggage key-value pairs
   */
  std::map<std::string, std::string> getAllEntries() const;

  /**
   * Get underlying W3C baggage if available.
   * @return W3C Baggage or nullopt if not available
   */
  absl::optional<W3C::Baggage> getW3CBaggage() const;

private:
  absl::optional<W3C::Baggage> w3c_baggage_;
};

} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
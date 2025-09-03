#pragma once

#include <string>
#include <vector>

#include "envoy/api/api.h"
#include "envoy/config/trace/v3/opentelemetry.pb.h"
#include "envoy/tracing/trace_context.h"

#include "source/extensions/propagators/opentelemetry/trace_context.h"
#include "source/extensions/propagators/w3c/propagator.h"
#include "source/extensions/propagators/b3/propagator.h"
#include "source/common/tracing/trace_context_impl.h"

#include "absl/status/statusor.h"
#include "absl/types/optional.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace OpenTelemetry {

// Constants for configuration
constexpr absl::string_view kOtelPropagatorsEnv = "OTEL_PROPAGATORS";
constexpr absl::string_view kDefaultPropagator = "tracecontext";

/**
 * Supported propagator types for OpenTelemetry configuration
 */
enum class PropagatorType {
  TraceContext,  // W3C Trace Context
  Baggage,       // W3C Baggage  
  B3,            // B3 single header
  B3Multi,       // B3 multiple headers
  None           // No propagation
};

/**
 * OpenTelemetry Composite Propagator implements the complete OpenTelemetry specification
 * for propagator composition, allowing multiple trace formats to be handled
 * through a single, unified interface with full specification compliance.
 * 
 * OPENTELEMETRY SPECIFICATION COMPLIANCE:
 * Official References:
 * - https://opentelemetry.io/docs/specs/otel/context/api-propagators/
 * - https://opentelemetry.io/docs/languages/sdk-configuration/general/#otel_propagators
 * 
 * Configuration Compliance:
 *    - OTEL_PROPAGATORS environment variable support with precedence
 *    - Default behavior: "tracecontext" only (per specification)
 *    - Supported propagators: "tracecontext", "baggage", "b3", "b3multi", "none"
 *    - Case-insensitive propagator names
 *    - Graceful handling of unknown/duplicate propagator names
 * 
 * Extraction Behavior Compliance:
 *    - Priority-based extraction: tries propagators in exact configuration order
 *    - First-match-wins: returns context from first successful propagator
 *    - Format-specific behavior: "b3" uses single header, "b3multi" uses multiple headers
 *    - No format mixing: does not merge contexts from different propagators
 *    - Graceful error handling with fallback through propagator list
 * 
 * Injection Behavior Compliance:
 *    - Multi-propagator injection: injects headers for ALL configured propagators
 *    - Format distinction: proper single vs multiple B3 header injection
 *    - Independent header management: each propagator manages its own headers
 *    - "none" propagator: completely disables propagation and clears headers
 * 
 * Composite Features:
 *    - Complete W3C Trace Context and Baggage support
 *    - Full B3 single and multiple header format support
 *    - Automatic format detection and conversion
 *    - Thread-safe operations and IoC pattern implementation
 *    - Backward compatibility with existing Envoy tracers
 * 
 * This eliminates code duplication across Envoy tracers while providing
 * comprehensive distributed tracing capabilities per OpenTelemetry standards.
 */
class Propagator {
public:
  /**
   * Configuration for the composite propagator.
   */
  struct Config {
    bool enable_baggage = true;
    bool strict_validation = false; // If true, fail on any validation errors
    std::vector<PropagatorType> propagators; // List of enabled propagators in priority order
  };

  /**
   * Checks if any supported trace headers are present.
   * @param trace_context The trace context to check
   * @return true if W3C or B3 headers are found
   */
  static bool isPresent(const Tracing::TraceContext& trace_context);

  /**
   * Extracts composite trace context from HTTP headers.
   * Tries W3C format first, then B3 as fallback.
   * @param trace_context The trace context containing headers
   * @return CompositeTraceContext or error status if no valid headers found
   */
  static absl::StatusOr<CompositeTraceContext> extract(const Tracing::TraceContext& trace_context);

  /**
   * Extracts composite trace context with configuration.
   * @param trace_context The trace context containing headers
   * @param config Propagator configuration
   * @return CompositeTraceContext or error status
   */
  static absl::StatusOr<CompositeTraceContext> extract(const Tracing::TraceContext& trace_context,
                                                       const Config& config);

  /**
   * Injects composite trace context into HTTP headers.
   * Uses default configuration (W3C primary format).
   * @param composite_context The composite trace context to inject
   * @param trace_context The target trace context for header injection
   * @return Success status or error if injection fails
   */
  static absl::Status inject(const CompositeTraceContext& composite_context,
                           Tracing::TraceContext& trace_context);

  /**
   * Injects composite trace context with configuration.
   * @param composite_context The composite trace context to inject
   * @param trace_context The target trace context for header injection
   * @param config Propagator configuration
   * @return Success status or error if injection fails
   */
  static absl::Status inject(const CompositeTraceContext& composite_context,
                           Tracing::TraceContext& trace_context,
                           const Config& config);

  /**
   * Extracts baggage from supported formats.
   * Currently only W3C format supports baggage.
   * @param trace_context The trace context containing headers
   * @return CompositeBaggage or error status
   */
  static absl::StatusOr<CompositeBaggage> extractBaggage(const Tracing::TraceContext& trace_context);

  /**
   * Injects baggage into supported formats.
   * @param baggage The baggage to inject
   * @param trace_context The target trace context
   * @return Success status or error if injection fails
   */
  static absl::Status injectBaggage(const CompositeBaggage& baggage,
                                  Tracing::TraceContext& trace_context);

  /**
   * Creates a new root trace context in the specified format.
   * @param trace_id The trace ID (hex string)
   * @param span_id The span ID (hex string)
   * @param sampled Whether the trace is sampled
   * @param format The desired trace format
   * @return CompositeTraceContext in the specified format
   */
  static absl::StatusOr<CompositeTraceContext> createRoot(absl::string_view trace_id,
                                                         absl::string_view span_id,
                                                         bool sampled,
                                                         TraceFormat format = TraceFormat::W3C);

  /**
   * Creates a child trace context from parent.
   * @param parent_context The parent composite trace context
   * @param new_span_id The new span ID (hex string)
   * @return Child CompositeTraceContext
   */
  static absl::StatusOr<CompositeTraceContext> createChild(const CompositeTraceContext& parent_context,
                                                          absl::string_view new_span_id);

  /**
   * Creates configuration from OpenTelemetry proto config with environment variable support.
   * Implements OTEL_PROPAGATORS environment variable specification.
   * @param otel_config The OpenTelemetry tracer configuration
   * @param api API interface for reading environment variables
   * @return Configuration with parsed propagator settings
   */
  static Config createConfig(const envoy::config::trace::v3::OpenTelemetryConfig& otel_config, 
                           Api::Api& api);

  /**
   * Creates configuration with explicit propagator list.
   * @param propagators List of propagator types in priority order
   * @param enable_baggage Whether to enable baggage propagation
   * @return Configuration
   */
  static Config createConfig(const std::vector<PropagatorType>& propagators,
                           bool enable_baggage = true);

private:
  /**
   * Extract trace context using configured propagators.
   */
  static absl::StatusOr<CompositeTraceContext> extractWithPropagators(
      const Tracing::TraceContext& trace_context,
      const std::vector<PropagatorType>& propagators,
      bool strict_validation);

  /**
   * Extract trace context using default behavior (W3C then B3).
   */
  static absl::StatusOr<CompositeTraceContext> extractWithDefaults(
      const Tracing::TraceContext& trace_context,
      bool strict_validation);

  /**
   * Clear all propagation headers when "none" is specified.
   */
  static absl::Status clearAllPropagationHeaders(Tracing::TraceContext& trace_context);

  /**
   * Tries to extract W3C trace context.
   * @param trace_context The trace context containing headers
   * @return CompositeTraceContext or nullopt if not found/invalid
   */
  static absl::optional<CompositeTraceContext> tryExtractW3C(const Tracing::TraceContext& trace_context);

  /**
   * Tries to extract B3 trace context using single header format.
   * @param trace_context The trace context containing headers
   * @return CompositeTraceContext or nullopt if not found/invalid
   */
  static absl::optional<CompositeTraceContext> tryExtractB3Single(const Tracing::TraceContext& trace_context);

  /**
   * Tries to extract B3 trace context using multiple headers format.
   * @param trace_context The trace context containing headers
   * @return CompositeTraceContext or nullopt if not found/invalid
   */
  static absl::optional<CompositeTraceContext> tryExtractB3Multiple(const Tracing::TraceContext& trace_context);

  /**
   * Tries to extract B3 trace context (auto-detect format).
   * @param trace_context The trace context containing headers
   * @return CompositeTraceContext or nullopt if not found/invalid
   */
  static absl::optional<CompositeTraceContext> tryExtractB3(const Tracing::TraceContext& trace_context);

  /**
   * Injects trace context in W3C format.
   * @param composite_context The composite context to inject
   * @param trace_context The target trace context
   * @return Success status or error
   */
  static absl::Status injectW3C(const CompositeTraceContext& composite_context,
                               Tracing::TraceContext& trace_context);

  /**
   * Injects trace context in B3 single header format.
   * @param composite_context The composite context to inject
   * @param trace_context The target trace context
   * @return Success status or error
   */
  static absl::Status injectB3Single(const CompositeTraceContext& composite_context,
                                   Tracing::TraceContext& trace_context);

  /**
   * Injects trace context in B3 multiple headers format.
   * @param composite_context The composite context to inject
   * @param trace_context The target trace context
   * @return Success status or error
   */
  static absl::Status injectB3Multiple(const CompositeTraceContext& composite_context,
                                     Tracing::TraceContext& trace_context);

  /**
   * Injects trace context in B3 format (auto-select format).
   * @param composite_context The composite context to inject
   * @param trace_context The target trace context
   * @return Success status or error
   */
  static absl::Status injectB3(const CompositeTraceContext& composite_context,
                              Tracing::TraceContext& trace_context);

  /**
   * Parse propagator configuration from proto and environment.
   */
  static std::vector<PropagatorType> parsePropagatorConfig(
      const envoy::config::trace::v3::OpenTelemetryConfig& config, Api::Api& api);

  /**
   * Extract propagator strings from environment variable and config.
   */
  static std::vector<std::string> extractPropagatorStrings(
      const envoy::config::trace::v3::OpenTelemetryConfig& config, Api::Api& api);

  /**
   * Convert string to propagator type.
   */
  static absl::StatusOr<PropagatorType> stringToPropagatorType(const std::string& propagator_str);

  /**
   * Check if propagator type is valid for trace context extraction.
   */
  static bool isTraceContextPropagator(PropagatorType type);
};

/**
 * TracingHelper provides backward compatibility interface for existing 
 * OpenTelemetry tracer to seamlessly integrate with the composite propagator.
 * 
 * This class eliminates the need to modify existing OpenTelemetry tracer code
 * while providing access to improved multi-format trace propagation.
 */
class TracingHelper {
public:
  /**
   * Configuration options for tracer integration.
   */
  struct TracerConfig {
    TraceFormat preferred_format = TraceFormat::W3C;
    bool enable_format_fallback = true;
    bool enable_baggage = true;
  };

  /**
   * Extracts trace context specifically optimized for OpenTelemetry tracer use.
   * Provides the same interface as the existing SpanContextExtractor.
   * @param trace_context The trace context containing headers
   * @return CompositeTraceContext or nullopt if not present
   */
  static absl::optional<CompositeTraceContext> extractForTracer(
      const Tracing::TraceContext& trace_context);

  /**
   * Extracts trace context with tracer-specific configuration.
   * @param trace_context The trace context containing headers
   * @param config Tracer configuration
   * @return CompositeTraceContext or nullopt if not present
   */
  static absl::optional<CompositeTraceContext> extractForTracer(
      const Tracing::TraceContext& trace_context,
      const TracerConfig& config);

  /**
   * Injects trace context from tracer-generated data.
   * @param composite_context The composite context to inject
   * @param trace_context The target trace context
   * @return Success status
   */
  static absl::Status injectFromTracer(const CompositeTraceContext& composite_context,
                                     Tracing::TraceContext& trace_context);

  /**
   * Injects with tracer configuration.
   * @param composite_context The composite context to inject
   * @param trace_context The target trace context
   * @param config Tracer configuration
   * @return Success status
   */
  static absl::Status injectFromTracer(const CompositeTraceContext& composite_context,
                                     Tracing::TraceContext& trace_context,
                                     const TracerConfig& config);

  /**
   * Checks if propagation headers are present (backward compatibility).
   * @param trace_context The trace context to check
   * @return true if any supported headers are found
   */
  static bool propagationHeaderPresent(const Tracing::TraceContext& trace_context);

  /**
   * Creates composite context from individual tracer values.
   * @param trace_id The trace ID (hex string)
   * @param span_id The span ID (hex string)
   * @param parent_span_id The parent span ID (hex string, empty if none)
   * @param sampled Whether the trace is sampled
   * @param trace_state Optional trace state (W3C only)
   * @param format Preferred format
   * @return CompositeTraceContext
   */
  static absl::StatusOr<CompositeTraceContext> createFromTracerData(
      absl::string_view trace_id,
      absl::string_view span_id,
      absl::string_view parent_span_id,
      bool sampled,
      absl::string_view trace_state = "",
      TraceFormat format = TraceFormat::W3C);

  /**
   * Create W3C trace context from tracer data.
   */
  static absl::StatusOr<CompositeTraceContext> createW3CFromTracerData(
      absl::string_view trace_id,
      absl::string_view span_id,
      absl::string_view parent_span_id,
      bool sampled,
      absl::string_view trace_state);

  /**
   * Create B3 trace context from tracer data.
   */
  static absl::StatusOr<CompositeTraceContext> createB3FromTracerData(
      absl::string_view trace_id,
      absl::string_view span_id,
      absl::string_view parent_span_id,
      bool sampled);

  /**
   * Extracts trace context using configurable propagators specifically for OpenTelemetry tracer.
   * This replaces the functionality previously in PropagatorConfig.
   * @param trace_context The trace context containing headers
   * @param config Propagator configuration from OpenTelemetry tracer config
   * @return CompositeTraceContext or error if no valid headers found
   */
  static absl::StatusOr<CompositeTraceContext> extractWithConfig(
      const Tracing::TraceContext& trace_context,
      const Config& config);

  /**
   * Checks if propagation headers are present using configurable propagators.
   * @param trace_context The trace context to check
   * @param config Propagator configuration
   * @return true if any configured propagators find headers
   */
  static bool propagationHeaderPresent(const Tracing::TraceContext& trace_context,
                                      const Config& config);

  /**
   * Check if specific propagator type has headers present.
   */
  static bool isPropagatorHeaderPresent(const Tracing::TraceContext& trace_context,
                                      PropagatorType propagator_type);

  /**
   * Injects composite trace context using configurable propagators.
   * @param composite_context The composite context to inject
   * @param trace_context The target trace context
   * @param config Propagator configuration
   * @return Success status or error if injection fails
   */
  static absl::Status injectWithConfig(const CompositeTraceContext& composite_context,
                                     Tracing::TraceContext& trace_context,
                                     const Config& config);
};

/**
 * PropagatorService encapsulates all propagation logic with configured propagators.
 * This service implements the IoC pattern, handling propagation internally without
 * exposing configuration details to clients.
 */
class PropagatorService {
public:
  /**
   * Creates a propagator service with the specified configuration.
   * @param config Propagator configuration containing enabled propagators
   */
  explicit PropagatorService(const Config& config);

  /**
   * Copy constructor for creating instances in TLS.
   */
  PropagatorService(const PropagatorService& other) = default;

  /**
   * Checks if any configured propagation headers are present.
   * @param trace_context The trace context to check
   * @return true if any configured propagators find headers
   */
  bool isPresent(const Tracing::TraceContext& trace_context) const;

  /**
   * Extracts trace context using configured propagators.
   * @param trace_context The trace context containing headers
   * @return CompositeTraceContext or error if no valid headers found
   */
  absl::StatusOr<CompositeTraceContext> extract(const Tracing::TraceContext& trace_context) const;

  /**
   * Injects trace context using configured propagators.
   * @param composite_context The composite context to inject
   * @param trace_context The target trace context
   * @return Success status or error if injection fails
   */
  absl::Status inject(const CompositeTraceContext& composite_context,
                     Tracing::TraceContext& trace_context) const;

  /**
   * Extracts baggage using configured propagators.
   * @param trace_context The trace context containing headers
   * @return CompositeBaggage or error status
   */
  absl::StatusOr<CompositeBaggage> extractBaggage(const Tracing::TraceContext& trace_context) const;

  /**
   * Injects baggage using configured propagators.
   * @param baggage The baggage to inject
   * @param trace_context The target trace context
   * @return Success status or error if injection fails
   */
  absl::Status injectBaggage(const CompositeBaggage& baggage,
                           Tracing::TraceContext& trace_context) const;

  /**
   * Extract baggage value by key for tracer getBaggage() implementation.
   * @param trace_context The trace context containing headers
   * @param key The baggage key to look up
   * @return The baggage value if found, empty string otherwise
   */
  std::string getBaggageValue(const Tracing::TraceContext& trace_context, 
                            absl::string_view key) const;

  /**
   * Set baggage value for tracer setBaggage() implementation.
   * @param trace_context The trace context to modify
   * @param key The baggage key
   * @param value The baggage value
   * @return true if successfully set, false if size limits exceeded
   */
  bool setBaggageValue(Tracing::TraceContext& trace_context,
                     absl::string_view key, absl::string_view value) const;

  /**
   * Creates from tracer data for injection.
   * @param trace_id The trace ID (hex string)
   * @param span_id The span ID (hex string)
   * @param parent_span_id The parent span ID (hex string, empty if none)
   * @param sampled Whether the trace is sampled
   * @param trace_state Optional trace state (W3C only)
   * @return CompositeTraceContext
   */
  absl::StatusOr<CompositeTraceContext> createFromTracerData(
      absl::string_view trace_id,
      absl::string_view span_id,
      absl::string_view parent_span_id,
      bool sampled,
      absl::string_view trace_state = "") const;

private:
  const Config config_;
};

using PropagatorServicePtr = std::unique_ptr<PropagatorService>;

/**
 * BaggageHelper provides integration with standard Span baggage interface
 * for composite propagator baggage support.
 */
class BaggageHelper {
public:
  /**
   * Extract baggage value by key for tracer getBaggage() implementation.
   * @param trace_context The trace context containing headers
   * @param key The baggage key to look up
   * @return The baggage value if found, empty string otherwise
   */
  static std::string getBaggageValue(const Tracing::TraceContext& trace_context, 
                                   absl::string_view key);

  /**
   * Set baggage value for tracer setBaggage() implementation.
   * @param trace_context The trace context to modify
   * @param key The baggage key
   * @param value The baggage value
   * @return true if successfully set, false if size limits exceeded
   */
  static bool setBaggageValue(Tracing::TraceContext& trace_context,
                            absl::string_view key, absl::string_view value);

  /**
   * Get all baggage as a map.
   * @param trace_context The trace context containing headers
   * @return Map of all baggage key-value pairs
   */
  static std::map<std::string, std::string> getAllBaggage(const Tracing::TraceContext& trace_context);

  /**
   * Check if any baggage is present.
   * @param trace_context The trace context to check
   * @return true if baggage header is present and valid
   */
  static bool hasBaggage(const Tracing::TraceContext& trace_context);
};

} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
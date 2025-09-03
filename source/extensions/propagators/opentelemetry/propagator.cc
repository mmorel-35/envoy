#include "source/extensions/propagators/opentelemetry/propagator.h"

#include "source/extensions/propagators/w3c/propagator.h"
#include "source/extensions/propagators/b3/propagator.h"
#include "source/common/common/logger.h"
#include "source/common/common/utility.h"
#include "source/common/config/datasource.h"

#include "absl/strings/str_format.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace OpenTelemetry {

// Propagator implementation

bool Propagator::isPresent(const Tracing::TraceContext& trace_context) {
  return W3C::Propagator::isPresent(trace_context) || B3::Propagator::isPresent(trace_context);
}

absl::StatusOr<CompositeTraceContext> Propagator::extract(const Tracing::TraceContext& trace_context) {
  Config default_config;
  // Default to W3C first, then B3 for backward compatibility
  default_config.propagators = {PropagatorType::TraceContext, PropagatorType::B3};
  return extract(trace_context, default_config);
}

absl::StatusOr<CompositeTraceContext> Propagator::extract(const Tracing::TraceContext& trace_context,
                                                         const Config& config) {
  // If no propagators configured, use default behavior
  if (config.propagators.empty()) {
    // Try W3C format first (preferred standard)
    auto w3c_result = tryExtractW3C(trace_context);
    if (w3c_result.has_value()) {
      return w3c_result.value();
    }
    
    // Try B3 format as fallback
    auto b3_result = tryExtractB3(trace_context);
    if (b3_result.has_value()) {
      return b3_result.value();
    }
  } else {
    // Try each configured propagator in priority order
    for (const auto& propagator_type : config.propagators) {
      switch (propagator_type) {
        case PropagatorType::TraceContext: {
          auto w3c_result = tryExtractW3C(trace_context);
          if (w3c_result.has_value()) {
            return w3c_result.value();
          }
          break;
        }
        case PropagatorType::B3:
        case PropagatorType::B3Multi: {
          auto b3_result = tryExtractB3(trace_context);
          if (b3_result.has_value()) {
            return b3_result.value();
          }
          break;
        }
        case PropagatorType::Baggage:
          // Baggage doesn't contain trace context, skip for span context extraction
          break;
        case PropagatorType::None:
          // No propagation
          break;
      }
    }
  }
  
  // No valid trace context found
  if (config.strict_validation) {
    return absl::InvalidArgumentError("No valid trace headers found in any supported format");
  } else {
    return absl::NotFoundError("No trace headers found");
  }
}

absl::Status Propagator::inject(const CompositeTraceContext& composite_context,
                               Tracing::TraceContext& trace_context) {
  Config default_config;
  return inject(composite_context, trace_context, default_config);
}

absl::Status Propagator::inject(const CompositeTraceContext& composite_context,
                               Tracing::TraceContext& trace_context,
                               const Config& config) {
  if (!composite_context.isValid()) {
    return absl::InvalidArgumentError("Cannot inject invalid trace context");
  }
  
  switch (config.injection_format) {
    case InjectionFormat::W3C_ONLY: {
      return injectW3C(composite_context, trace_context);
    }
    case InjectionFormat::B3_ONLY: {
      return injectB3(composite_context, trace_context);
    }
    case InjectionFormat::W3C_PRIMARY: {
      auto w3c_status = injectW3C(composite_context, trace_context);
      if (!w3c_status.ok() && composite_context.format() != TraceFormat::W3C) {
        // Fallback to B3 if W3C injection fails and we're not already in W3C format
        return injectB3(composite_context, trace_context);
      }
      return w3c_status;
    }
    case InjectionFormat::B3_PRIMARY: {
      auto b3_status = injectB3(composite_context, trace_context);
      if (!b3_status.ok() && composite_context.format() != TraceFormat::B3) {
        // Fallback to W3C if B3 injection fails and we're not already in B3 format
        return injectW3C(composite_context, trace_context);
      }
      return b3_status;
    }
    case InjectionFormat::BOTH: {
      auto w3c_status = injectW3C(composite_context, trace_context);
      auto b3_status = injectB3(composite_context, trace_context);
      
      // Return error only if both injections fail
      if (!w3c_status.ok() && !b3_status.ok()) {
        return absl::InvalidArgumentError(absl::StrFormat(
            "Both W3C and B3 injection failed: W3C=%s, B3=%s",
            w3c_status.ToString(), b3_status.ToString()));
      }
      return absl::OkStatus();
    }
    default:
      return absl::InvalidArgumentError("Unknown injection format");
  }
}

absl::StatusOr<CompositeBaggage> Propagator::extractBaggage(const Tracing::TraceContext& trace_context) {
  // Only W3C supports baggage currently
  if (W3C::Propagator::isBaggagePresent(trace_context)) {
    auto w3c_baggage_result = W3C::Propagator::extractBaggage(trace_context);
    if (w3c_baggage_result.ok()) {
      return CompositeBaggage(w3c_baggage_result.value());
    }
  }
  
  // Return empty baggage if none found
  return CompositeBaggage();
}

absl::Status Propagator::injectBaggage(const CompositeBaggage& baggage,
                                     Tracing::TraceContext& trace_context) {
  auto w3c_baggage = baggage.getW3CBaggage();
  if (w3c_baggage.has_value()) {
    W3C::Propagator::injectBaggage(w3c_baggage.value(), trace_context);
  }
  return absl::OkStatus();
}

absl::StatusOr<CompositeTraceContext> Propagator::createRoot(absl::string_view trace_id,
                                                           absl::string_view span_id,
                                                           bool sampled,
                                                           TraceFormat format) {
  switch (format) {
    case TraceFormat::W3C: {
      auto w3c_result = W3C::Propagator::createRoot(trace_id, span_id, sampled);
      if (!w3c_result.ok()) {
        return w3c_result.status();
      }
      return CompositeTraceContext(w3c_result.value());
    }
    case TraceFormat::B3: {
      // Parse trace ID and span ID for B3
      auto trace_id_result = B3::TraceId::fromHexString(trace_id);
      if (!trace_id_result.ok()) {
        return trace_id_result.status();
      }
      
      auto span_id_result = B3::SpanId::fromHexString(span_id);
      if (!span_id_result.ok()) {
        return span_id_result.status();
      }
      
      B3::SamplingState sampling_state = sampled ? B3::SamplingState::SAMPLED : B3::SamplingState::NOT_SAMPLED;
      
      B3::TraceContext b3_ctx(trace_id_result.value(), span_id_result.value(), 
                             absl::nullopt, sampling_state);
      
      return CompositeTraceContext(b3_ctx);
    }
    case TraceFormat::NONE:
    default:
      return absl::InvalidArgumentError("Invalid format for creating root context");
  }
}

absl::StatusOr<CompositeTraceContext> Propagator::createChild(const CompositeTraceContext& parent_context,
                                                            absl::string_view new_span_id) {
  return parent_context.createChild(new_span_id);
}

absl::optional<CompositeTraceContext> Propagator::tryExtractW3C(const Tracing::TraceContext& trace_context) {
  if (!W3C::Propagator::isPresent(trace_context)) {
    return absl::nullopt;
  }
  
  auto w3c_result = W3C::Propagator::extract(trace_context);
  if (!w3c_result.ok()) {
    return absl::nullopt;
  }
  
  return CompositeTraceContext(w3c_result.value());
}

absl::optional<CompositeTraceContext> Propagator::tryExtractB3(const Tracing::TraceContext& trace_context) {
  if (!B3::Propagator::isPresent(trace_context)) {
    return absl::nullopt;
  }
  
  auto b3_result = B3::Propagator::extract(trace_context);
  if (!b3_result.ok()) {
    return absl::nullopt;
  }
  
  return CompositeTraceContext(b3_result.value());
}

absl::Status Propagator::injectW3C(const CompositeTraceContext& composite_context,
                                  Tracing::TraceContext& trace_context) {
  W3C::TraceContext w3c_context;
  
  if (composite_context.format() == TraceFormat::W3C) {
    // Direct injection from W3C context
    auto w3c_ctx = composite_context.getW3CContext();
    if (!w3c_ctx.has_value()) {
      return absl::InternalError("W3C context not available");
    }
    w3c_context = w3c_ctx.value();
  } else {
    // Convert from other format to W3C
    auto converted_result = composite_context.convertTo(TraceFormat::W3C);
    if (!converted_result.ok()) {
      return converted_result.status();
    }
    
    auto w3c_ctx = converted_result.value().getW3CContext();
    if (!w3c_ctx.has_value()) {
      return absl::InternalError("Conversion to W3C failed");
    }
    w3c_context = w3c_ctx.value();
  }
  
  W3C::Propagator::inject(w3c_context, trace_context);
  return absl::OkStatus();
}

absl::Status Propagator::injectB3(const CompositeTraceContext& composite_context,
                                 Tracing::TraceContext& trace_context) {
  B3::TraceContext b3_context;
  
  if (composite_context.format() == TraceFormat::B3) {
    // Direct injection from B3 context
    auto b3_ctx = composite_context.getB3Context();
    if (!b3_ctx.has_value()) {
      return absl::InternalError("B3 context not available");
    }
    b3_context = b3_ctx.value();
  } else {
    // Convert from other format to B3
    auto converted_result = composite_context.convertTo(TraceFormat::B3);
    if (!converted_result.ok()) {
      return converted_result.status();
    }
    
    auto b3_ctx = converted_result.value().getB3Context();
    if (!b3_ctx.has_value()) {
      return absl::InternalError("Conversion to B3 failed");
    }
    b3_context = b3_ctx.value();
  }
  
  return B3::Propagator::inject(b3_context, trace_context);
}

// TracingHelper implementation

absl::optional<CompositeTraceContext> TracingHelper::extractForTracer(
    const Tracing::TraceContext& trace_context) {
  TracerConfig default_config;
  return extractForTracer(trace_context, default_config);
}

absl::optional<CompositeTraceContext> TracingHelper::extractForTracer(
    const Tracing::TraceContext& trace_context,
    const TracerConfig& config) {
  
  if (config.preferred_format == TraceFormat::W3C) {
    // Try W3C first
    auto w3c_result = Propagator::tryExtractW3C(trace_context);
    if (w3c_result.has_value()) {
      return w3c_result.value();
    }
    
    // Try B3 as fallback if enabled
    if (config.enable_format_fallback) {
      auto b3_result = Propagator::tryExtractB3(trace_context);
      if (b3_result.has_value()) {
        return b3_result.value();
      }
    }
  } else if (config.preferred_format == TraceFormat::B3) {
    // Try B3 first
    auto b3_result = Propagator::tryExtractB3(trace_context);
    if (b3_result.has_value()) {
      return b3_result.value();
    }
    
    // Try W3C as fallback if enabled
    if (config.enable_format_fallback) {
      auto w3c_result = Propagator::tryExtractW3C(trace_context);
      if (w3c_result.has_value()) {
        return w3c_result.value();
      }
    }
  }
  
  return absl::nullopt;
}

absl::Status TracingHelper::injectFromTracer(const CompositeTraceContext& composite_context,
                                           Tracing::TraceContext& trace_context) {
  TracerConfig default_config;
  return injectFromTracer(composite_context, trace_context, default_config);
}

absl::Status TracingHelper::injectFromTracer(const CompositeTraceContext& composite_context,
                                           Tracing::TraceContext& trace_context,
                                           const TracerConfig& config) {
  Propagator::Config prop_config;
  
  // Map tracer config to propagator config
  if (config.preferred_format == TraceFormat::W3C) {
    prop_config.injection_format = Propagator::InjectionFormat::W3C_PRIMARY;
  } else if (config.preferred_format == TraceFormat::B3) {
    prop_config.injection_format = Propagator::InjectionFormat::B3_PRIMARY;
  } else {
    prop_config.injection_format = Propagator::InjectionFormat::W3C_PRIMARY; // Default
  }
  
  prop_config.enable_baggage = config.enable_baggage;
  
  return Propagator::inject(composite_context, trace_context, prop_config);
}

bool TracingHelper::propagationHeaderPresent(const Tracing::TraceContext& trace_context) {
  return Propagator::isPresent(trace_context);
}

absl::StatusOr<CompositeTraceContext> TracingHelper::createFromTracerData(
    absl::string_view trace_id,
    absl::string_view span_id,
    absl::string_view parent_span_id,
    bool sampled,
    absl::string_view trace_state,
    TraceFormat format) {
  
  switch (format) {
    case TraceFormat::W3C: {
      auto w3c_result = W3C::Propagator::createRoot(trace_id, span_id, sampled);
      if (!w3c_result.ok()) {
        return w3c_result.status();
      }
      
      auto w3c_ctx = w3c_result.value();
      
      // Set parent span ID if provided
      if (!parent_span_id.empty()) {
        w3c_ctx.mutableTraceparent().setParentId(parent_span_id);
      }
      
      // Set trace state if provided
      if (!trace_state.empty()) {
        auto tracestate_result = W3C::TraceState::fromString(trace_state);
        if (tracestate_result.ok()) {
          w3c_ctx.setTracestate(tracestate_result.value());
        }
      }
      
      return CompositeTraceContext(w3c_ctx);
    }
    case TraceFormat::B3: {
      // Parse IDs for B3
      auto trace_id_result = B3::TraceId::fromString(trace_id);
      if (!trace_id_result.ok()) {
        return trace_id_result.status();
      }
      
      auto span_id_result = B3::SpanId::fromString(span_id);
      if (!span_id_result.ok()) {
        return span_id_result.status();
      }
      
      absl::optional<B3::SpanId> parent_span_id_opt;
      if (!parent_span_id.empty()) {
        auto parent_result = B3::SpanId::fromString(parent_span_id);
        if (!parent_result.ok()) {
          return parent_result.status();
        }
        parent_span_id_opt = parent_result.value();
      }
      
      B3::SamplingState sampling_state = sampled ? B3::SamplingState::SAMPLED : B3::SamplingState::NOT_SAMPLED;
      
      B3::TraceContext b3_ctx(trace_id_result.value(), span_id_result.value(), 
                             parent_span_id_opt, sampling_state);
      
      return CompositeTraceContext(b3_ctx);
    }
    case TraceFormat::NONE:
    default:
      return absl::InvalidArgumentError("Invalid format for creating trace context");
  }
}

absl::StatusOr<CompositeTraceContext> TracingHelper::extractWithConfig(
    const Tracing::TraceContext& trace_context,
    const Config& config) {
  return Propagator::extract(trace_context, config);
}

bool TracingHelper::propagationHeaderPresent(const Tracing::TraceContext& trace_context,
                                           const Config& config) {
  // Check for each enabled propagator type
  for (const auto& propagator_type : config.propagators) {
    switch (propagator_type) {
      case PropagatorType::TraceContext:
        if (W3C::Propagator::isPresent(trace_context)) {
          return true;
        }
        break;
      case PropagatorType::B3:
      case PropagatorType::B3Multi:
        if (B3::Propagator::isPresent(trace_context)) {
          return true;
        }
        break;
      case PropagatorType::Baggage:
        if (W3C::Propagator::isBaggagePresent(trace_context)) {
          return true;
        }
        break;
      case PropagatorType::None:
        // No propagation
        break;
    }
  }
  return false;
}

absl::Status TracingHelper::injectWithConfig(const CompositeTraceContext& composite_context,
                                           Tracing::TraceContext& trace_context,
                                           const Config& config) {
  return Propagator::inject(composite_context, trace_context, config);
}

// BaggageHelper implementation

std::string BaggageHelper::getBaggageValue(const Tracing::TraceContext& trace_context,
                                         absl::string_view key) {
  auto baggage_result = Propagator::extractBaggage(trace_context);
  if (baggage_result.ok()) {
    return baggage_result.value().getValue(key);
  }
  return "";
}

bool BaggageHelper::setBaggageValue(Tracing::TraceContext& trace_context,
                                  absl::string_view key, absl::string_view value) {
  auto baggage_result = Propagator::extractBaggage(trace_context);
  CompositeBaggage baggage;
  
  if (baggage_result.ok()) {
    baggage = baggage_result.value();
  }
  
  if (baggage.setValue(key, value)) {
    auto inject_status = Propagator::injectBaggage(baggage, trace_context);
    return inject_status.ok();
  }
  
  return false;
}

std::map<std::string, std::string> BaggageHelper::getAllBaggage(const Tracing::TraceContext& trace_context) {
  auto baggage_result = Propagator::extractBaggage(trace_context);
  if (baggage_result.ok()) {
    return baggage_result.value().getAllEntries();
  }
  return {};
}

bool BaggageHelper::hasBaggage(const Tracing::TraceContext& trace_context) {
  auto baggage_result = Propagator::extractBaggage(trace_context);
  if (baggage_result.ok()) {
    return !baggage_result.value().isEmpty();
  }
  return false;
}

// Configuration implementation

constexpr absl::string_view kOtelPropagatorsEnv = "OTEL_PROPAGATORS";
constexpr absl::string_view kDefaultPropagator = "tracecontext";

Propagator::Config Propagator::createConfig(const envoy::config::trace::v3::OpenTelemetryConfig& otel_config, 
                                           Api::Api& api) {
  Config config;
  config.propagators = parsePropagatorConfig(otel_config, api);
  // Set injection format based on propagators priority
  if (!config.propagators.empty()) {
    switch (config.propagators[0]) {
      case PropagatorType::TraceContext:
        config.injection_format = InjectionFormat::W3C_PRIMARY;
        break;
      case PropagatorType::B3:
      case PropagatorType::B3Multi:
        config.injection_format = InjectionFormat::B3_PRIMARY;
        break;
      default:
        config.injection_format = InjectionFormat::W3C_PRIMARY;
        break;
    }
  }
  
  // Enable baggage if configured
  config.enable_baggage = std::find(config.propagators.begin(), config.propagators.end(), 
                                   PropagatorType::Baggage) != config.propagators.end();
  
  return config;
}

Propagator::Config Propagator::createConfig(const std::vector<PropagatorType>& propagators,
                                           InjectionFormat injection_format,
                                           bool enable_baggage) {
  Config config;
  config.propagators = propagators;
  config.injection_format = injection_format;
  config.enable_baggage = enable_baggage;
  return config;
}

std::vector<PropagatorType> Propagator::parsePropagatorConfig(
    const envoy::config::trace::v3::OpenTelemetryConfig& config, Api::Api& api) {
  std::vector<std::string> propagator_strings;

  // First, try environment variable
  envoy::config::core::v3::DataSource ds;
  ds.set_environment_variable(kOtelPropagatorsEnv);
  
  std::string env_propagators;
  TRY_NEEDS_AUDIT {
    env_propagators = THROW_OR_RETURN_VALUE(Config::DataSource::read(ds, true, api), std::string);
  }
  END_TRY catch (const EnvoyException& e) {
    // Environment variable not set or error reading, continue with config
  }

  if (!env_propagators.empty()) {
    // Parse comma-separated list from environment
    for (const auto& propagator : Envoy::StringUtil::splitToken(env_propagators, ",", false)) {
      propagator_strings.push_back(std::string(Envoy::StringUtil::trim(propagator)));
    }
  } else if (!config.propagators().empty()) {
    // Use configuration from proto
    for (const auto& propagator : config.propagators()) {
      propagator_strings.push_back(propagator);
    }
  } else {
    // Default to tracecontext for backward compatibility
    propagator_strings.push_back(std::string(kDefaultPropagator));
  }

  // Convert strings to propagator types
  std::vector<PropagatorType> propagators;
  for (const auto& propagator_str : propagator_strings) {
    auto propagator_type = stringToPropagatorType(propagator_str);
    if (propagator_type.ok()) {
      propagators.push_back(propagator_type.value());
      
      // Handle "none" special case
      if (propagator_type.value() == PropagatorType::None) {
        // Clear all propagators for "none"
        propagators.clear();
        break;
      }
    } else {
      ENVOY_LOG(warn, "Unknown propagator type '{}', ignoring", propagator_str);
    }
  }

  // Remove duplicates while preserving order
  auto last = std::unique(propagators.begin(), propagators.end());
  propagators.erase(last, propagators.end());
  
  return propagators;
}

absl::StatusOr<PropagatorType> Propagator::stringToPropagatorType(const std::string& propagator_str) {
  std::string lower_str = absl::AsciiStrToLower(propagator_str);
  
  if (lower_str == "tracecontext") {
    return PropagatorType::TraceContext;
  } else if (lower_str == "baggage") {
    return PropagatorType::Baggage;
  } else if (lower_str == "b3") {
    return PropagatorType::B3;
  } else if (lower_str == "b3multi") {
    return PropagatorType::B3Multi;
  } else if (lower_str == "none") {
    return PropagatorType::None;
  }
  
  return absl::InvalidArgumentError(absl::StrCat("Unknown propagator type: ", propagator_str));
}

} // namespace OpenTelemetry
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy
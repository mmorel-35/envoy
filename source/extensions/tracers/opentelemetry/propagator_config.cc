#include "source/extensions/tracers/opentelemetry/propagator_config.h"

#include <algorithm>

#include "source/common/common/logger.h"
#include "source/common/common/utility.h"
#include "source/common/config/datasource.h"

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

constexpr absl::string_view kOtelPropagatorsEnv = "OTEL_PROPAGATORS";
constexpr absl::string_view kDefaultPropagator = "tracecontext";

PropagatorConfig::PropagatorConfig(const envoy::config::trace::v3::OpenTelemetryConfig& config, Api::Api& api) 
    : trace_context_enabled_(false), baggage_enabled_(false), b3_enabled_(false), b3_multi_enabled_(false) {
  parsePropagatorConfig(config, api);
}

void PropagatorConfig::parsePropagatorConfig(const envoy::config::trace::v3::OpenTelemetryConfig& config, Api::Api& api) {
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
  for (const auto& propagator_str : propagator_strings) {
    auto propagator_type = stringToPropagatorType(propagator_str);
    if (propagator_type.ok()) {
      propagators_.push_back(propagator_type.value());
      
      // Set enabled flags for quick lookup
      switch (propagator_type.value()) {
        case PropagatorType::TraceContext:
          trace_context_enabled_ = true;
          break;
        case PropagatorType::Baggage:
          baggage_enabled_ = true;
          break;
        case PropagatorType::B3:
          b3_enabled_ = true;
          break;
        case PropagatorType::B3Multi:
          b3_multi_enabled_ = true;
          break;
        case PropagatorType::None:
          // Clear all propagators for "none"
          propagators_.clear();
          trace_context_enabled_ = false;
          baggage_enabled_ = false;
          b3_enabled_ = false;
          b3_multi_enabled_ = false;
          return;
      }
    } else {
      ENVOY_LOG(warn, "Unknown propagator type '{}', ignoring", propagator_str);
    }
  }

  // Remove duplicates while preserving order
  auto last = std::unique(propagators_.begin(), propagators_.end());
  propagators_.erase(last, propagators_.end());
}

absl::StatusOr<PropagatorType> PropagatorConfig::stringToPropagatorType(const std::string& propagator_str) {
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

bool PropagatorConfig::propagationHeaderPresent(const Tracing::TraceContext& trace_context) {
  // Check for each enabled propagator type
  if (trace_context_enabled_ && Extensions::Propagators::W3C::Propagator::isPresent(trace_context)) {
    return true;
  }
  
  if ((b3_enabled_ || b3_multi_enabled_) && Extensions::Propagators::B3::Propagator::isPresent(trace_context)) {
    return true;
  }
  
  return false;
}

absl::StatusOr<SpanContext> PropagatorConfig::extractSpanContext(const Tracing::TraceContext& trace_context) {
  // Try each propagator in priority order
  for (const auto& propagator_type : propagators_) {
    switch (propagator_type) {
      case PropagatorType::TraceContext: {
        auto w3c_result = Extensions::Propagators::W3C::Propagator::extract(trace_context);
        if (w3c_result.ok()) {
          return convertFromW3C(w3c_result.value());
        }
        break;
      }
      case PropagatorType::B3:
      case PropagatorType::B3Multi: {
        auto b3_result = Extensions::Propagators::B3::Propagator::extract(trace_context);
        if (b3_result.ok()) {
          return convertFromB3(b3_result.value());
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
  
  return absl::InvalidArgumentError("No supported propagation headers found");
}

void PropagatorConfig::injectSpanContext(const SpanContext& span_context, Tracing::TraceContext& trace_context) {
  // Inject using all enabled propagators
  for (const auto& propagator_type : propagators_) {
    switch (propagator_type) {
      case PropagatorType::TraceContext: {
        auto w3c_context = convertToW3C(span_context);
        Extensions::Propagators::W3C::Propagator::inject(w3c_context, trace_context);
        break;
      }
      case PropagatorType::B3:
      case PropagatorType::B3Multi: {
        auto b3_context = convertToB3(span_context);
        auto result = Extensions::Propagators::B3::Propagator::inject(b3_context, trace_context);
        if (!result.ok()) {
          ENVOY_LOG(warn, "Failed to inject B3 context: {}", result.message());
        }
        break;
      }
      case PropagatorType::Baggage:
        // For now, baggage injection is handled separately
        // TODO: Implement baggage injection when Span interface supports it
        break;
      case PropagatorType::None:
        // No injection
        break;
    }
  }
}

absl::StatusOr<SpanContext> PropagatorConfig::convertFromW3C(
    const Extensions::Propagators::W3C::TraceContext& w3c_context) {
  
  const auto& trace_parent = w3c_context.traceParent();
  const auto& trace_state = w3c_context.traceState();
  
  // Create OpenTelemetry SpanContext
  SpanContext span_context(
    trace_parent.version(),
    trace_parent.traceId(),
    trace_parent.spanId(),
    trace_parent.isSampled(),
    trace_state.toString()
  );
  
  return span_context;
}

absl::StatusOr<SpanContext> PropagatorConfig::convertFromB3(
    const Extensions::Propagators::B3::TraceContext& b3_context) {
  
  // Create OpenTelemetry SpanContext with version "00" for B3 compatibility
  SpanContext span_context(
    "00",                                    // version
    b3_context.traceId().toHexString(),     // trace_id
    b3_context.spanId().toHexString(),      // span_id
    b3_context.sampled(),                   // sampled (uses the sampled() method which handles debug too)
    ""                                      // trace_state (empty for B3)
  );
  
  return span_context;
}

Extensions::Propagators::W3C::TraceContext PropagatorConfig::convertToW3C(const SpanContext& span_context) {
  // Create W3C TraceParent
  Extensions::Propagators::W3C::TraceParent trace_parent(
    span_context.version(),
    span_context.trace_id(),
    span_context.span_id(),
    span_context.sampled()
  );
  
  // Create W3C TraceState
  Extensions::Propagators::W3C::TraceState trace_state;
  if (!span_context.trace_state().empty()) {
    auto parsed_state = Extensions::Propagators::W3C::TraceState::parse(span_context.trace_state());
    if (parsed_state.ok()) {
      trace_state = parsed_state.value();
    }
  }
  
  // Create empty baggage for now
  Extensions::Propagators::W3C::Baggage baggage;
  
  return Extensions::Propagators::W3C::TraceContext(trace_parent, trace_state, baggage);
}

Extensions::Propagators::B3::TraceContext PropagatorConfig::convertToB3(const SpanContext& span_context) {
  // Create B3 TraceId
  auto trace_id_result = Extensions::Propagators::B3::TraceId::fromHexString(span_context.trace_id());
  if (!trace_id_result.ok()) {
    // Return empty context if conversion fails
    return Extensions::Propagators::B3::TraceContext();
  }
  
  // Create B3 SpanId
  auto span_id_result = Extensions::Propagators::B3::SpanId::fromHexString(span_context.span_id());
  if (!span_id_result.ok()) {
    // Return empty context if conversion fails
    return Extensions::Propagators::B3::TraceContext();
  }
  
  // Create B3 SamplingState
  Extensions::Propagators::B3::SamplingState sampling_state = 
    span_context.sampled() ? 
    Extensions::Propagators::B3::SamplingState::SAMPLED : 
    Extensions::Propagators::B3::SamplingState::NOT_SAMPLED;
  
  return Extensions::Propagators::B3::TraceContext(
    trace_id_result.value(), 
    span_id_result.value(), 
    absl::nullopt, // no parent span id
    sampling_state,
    false // not debug
  );
}

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
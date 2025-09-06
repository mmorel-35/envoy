#include "source/extensions/tracers/fluentd/fluentd_tracer_impl.h"

#include <cstdint>

#include "source/common/buffer/buffer_impl.h"
#include "source/common/common/backoff_strategy.h"
#include "source/common/common/hex.h"
#include "source/common/tracing/trace_context_impl.h"

#include "msgpack.hpp"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace Fluentd {

using W3cConstants = Envoy::Extensions::Propagators::W3c::W3cConstants;
using TraceContextPropagator = Envoy::Extensions::Propagators::W3c::TraceContext::TraceContextPropagator;

SpanContextExtractor::SpanContextExtractor(Tracing::TraceContext& trace_context)
    : trace_context_(trace_context) {}

SpanContextExtractor::~SpanContextExtractor() = default;

bool SpanContextExtractor::propagationHeaderPresent() {
  TraceContextPropagator propagator;
  return propagator.hasTraceParent(trace_context_);
}

absl::StatusOr<SpanContext> SpanContextExtractor::extractSpanContext() {
  TraceContextPropagator propagator;
  
  // Extract traceparent using the W3C propagator
  auto traceparent = propagator.extractTraceParent(trace_context_);
  if (!traceparent.has_value()) {
    return absl::InvalidArgumentError("No traceparent header found");
  }

  // Parse using the W3C propagator
  auto parsed = propagator.parseTraceParent(traceparent.value());
  if (!parsed.ok()) {
    return parsed.status();
  }

  // Extract tracestate if present
  auto tracestate = propagator.extractTraceState(trace_context_);
  std::string tracestate_str = tracestate.value_or("");

  // Create SpanContext from parsed data
  SpanContext span_context(parsed->version, parsed->trace_id, parsed->span_id, 
                          parsed->sampled, tracestate_str);
  return span_context;
}

// Define default version and trace context construction
constexpr absl::string_view kDefaultVersion = "00";

// Initialize the Fluentd driver
Driver::Driver(const FluentdConfigSharedPtr fluentd_config,
               Server::Configuration::TracerFactoryContext& context,
               FluentdTracerCacheSharedPtr tracer_cache)
    : tls_slot_(context.serverFactoryContext().threadLocal().allocateSlot()),
      fluentd_config_(fluentd_config), tracer_cache_(tracer_cache) {
  Random::RandomGenerator& random = context.serverFactoryContext().api().randomGenerator();

  uint64_t base_interval_ms = DefaultBaseBackoffIntervalMs;
  uint64_t max_interval_ms = base_interval_ms * DefaultMaxBackoffIntervalFactor;

  if (fluentd_config->has_retry_policy() && fluentd_config->retry_policy().has_retry_back_off()) {
    base_interval_ms = PROTOBUF_GET_MS_OR_DEFAULT(fluentd_config->retry_policy().retry_back_off(),
                                                  base_interval, DefaultBaseBackoffIntervalMs);
    max_interval_ms =
        PROTOBUF_GET_MS_OR_DEFAULT(fluentd_config->retry_policy().retry_back_off(), max_interval,
                                   base_interval_ms * DefaultMaxBackoffIntervalFactor);
  }

  // Create a thread local tracer
  tls_slot_->set([fluentd_config = fluentd_config_, &random, tracer_cache = tracer_cache_,
                  base_interval_ms, max_interval_ms](Event::Dispatcher&) {
    BackOffStrategyPtr backoff_strategy = std::make_unique<JitteredExponentialBackOffStrategy>(
        base_interval_ms, max_interval_ms, random);
    return std::make_shared<ThreadLocalTracer>(
        tracer_cache->getOrCreate(fluentd_config, random, std::move(backoff_strategy)));
  });
}

// Handles driver logic for starting a new span
Tracing::SpanPtr Driver::startSpan(const Tracing::Config& /*config*/,
                                   Tracing::TraceContext& trace_context,
                                   const StreamInfo::StreamInfo& stream_info,
                                   const std::string& operation_name,
                                   Tracing::Decision tracing_decision) {
  // Get the thread local tracer
  auto& tracer = tls_slot_->getTyped<ThreadLocalTracer>().tracer();

  // Decide which tracer.startSpan function to call based on available span context
  SpanContextExtractor extractor(trace_context);
  if (!extractor.propagationHeaderPresent()) {
    // No propagation header, so we can create a fresh span with the given decision.
    return tracer.startSpan(stream_info.startTime(), operation_name, tracing_decision);
  } else {
    // Try to extract the span context. If we can't, just return a null span.
    absl::StatusOr<SpanContext> span_context = extractor.extractSpanContext();
    if (span_context.ok()) {
      return tracer.startSpan(stream_info.startTime(), operation_name, span_context.value());
    } else {
      ENVOY_LOG(trace, "Unable to extract span context: ", span_context.status());
      return std::make_unique<Tracing::NullSpan>();
    }
  }
}

// Initialize the Fluentd tracer
FluentdTracerImpl::FluentdTracerImpl(Upstream::ThreadLocalCluster& cluster,
                                     Tcp::AsyncTcpClientPtr client, Event::Dispatcher& dispatcher,
                                     const FluentdConfig& config,
                                     BackOffStrategyPtr backoff_strategy,
                                     Stats::Scope& parent_scope, Random::RandomGenerator& random)
    : FluentdBase(
          cluster, std::move(client), dispatcher, config.tag(),
          config.has_retry_policy() && config.retry_policy().has_num_retries()
              ? absl::optional<uint32_t>(config.retry_policy().num_retries().value())
              : absl::nullopt,
          parent_scope, config.stat_prefix(), std::move(backoff_strategy),
          PROTOBUF_GET_MS_OR_DEFAULT(config, buffer_flush_interval, DefaultBufferFlushIntervalMs),
          PROTOBUF_GET_WRAPPED_OR_DEFAULT(config, buffer_size_bytes, DefaultMaxBufferSize)),
      option_({{"fluent_signal", "2"}, {"TimeFormat", "DateTime"}}), random_(random),
      time_source_(dispatcher.timeSource()) {}

// Initialize a span object
Span::Span(SystemTime start_time, const std::string& operation_name, FluentdTracerSharedPtr tracer,
           SpanContext&& span_context, TimeSource& time_source, bool use_local_decision)
    : start_time_(start_time), operation_(operation_name), tracer_(std::move(tracer)),
      span_context_(std::move(span_context)), time_source_(time_source),
      use_local_decision_(use_local_decision) {}

// Set the operation name for the span
void Span::setOperation(absl::string_view operation) { operation_ = std::string(operation); }

// Adds a tag to the span
void Span::setTag(absl::string_view name, absl::string_view value) {
  tags_[std::string(name)] = std::string(value);
}

// Log an event as a Fluentd entry
void Span::log(SystemTime /*timestamp*/, const std::string& event) {
  uint64_t time =
      std::chrono::duration_cast<std::chrono::seconds>(time_source_.systemTime().time_since_epoch())
          .count();

  EntryPtr entry =
      std::make_unique<Entry>(time, std::map<std::string, std::string>{{"event", event}});

  tracer_->log(std::move(entry));
}

// Finish and log a span as a Fluentd entry
void Span::finishSpan() {
  uint64_t time =
      std::chrono::duration_cast<std::chrono::seconds>(time_source_.systemTime().time_since_epoch())
          .count();

  // Make the record map
  std::map<std::string, std::string> record_map = std::move(tags_);
  record_map["operation"] = operation_;
  record_map["trace_id"] = span_context_.traceId();
  record_map["span_id"] = span_context_.spanId();
  record_map["start_time"] = std::to_string(
      std::chrono::duration_cast<std::chrono::seconds>(start_time_.time_since_epoch()).count());
  record_map["end_time"] = std::to_string(time);

  EntryPtr entry = std::make_unique<Entry>(time, std::move(record_map));

  tracer_->log(std::move(entry));
}

// Inject the span context into the trace context
void Span::injectContext(Tracing::TraceContext& trace_context,
                         const Tracing::UpstreamContext& /*upstream*/) {
  
  // Use the W3C TraceContext propagator for proper W3C header injection
  TraceContextPropagator propagator;
  
  // Inject traceparent using the propagator
  propagator.injectTraceParent(trace_context, span_context_.version(),
                              span_context_.traceId(), span_context_.spanId(),
                              span_context_.sampled());
  
  // Inject tracestate if present
  if (!span_context_.tracestate().empty()) {
    propagator.injectTraceState(trace_context, span_context_.tracestate());
  }
}

// Spawns a child span
Tracing::SpanPtr Span::spawnChild(const Tracing::Config&, const std::string& name,
                                  SystemTime start_time) {
  return tracer_->startSpan(start_time, name, span_context_);
}

std::string Span::getBaggage(absl::string_view /*key*/) {
  // not implemented
  return EMPTY_STRING;
}

void Span::setBaggage(absl::string_view /*key*/, absl::string_view /*value*/) {
  // not implemented
}

std::string Span::getTraceId() const { return span_context_.traceId(); }

std::string Span::getSpanId() const { return span_context_.spanId(); }

// Start a new span with no parent context
Tracing::SpanPtr FluentdTracerImpl::startSpan(SystemTime start_time,
                                              const std::string& operation_name,
                                              Tracing::Decision tracing_decision) {
  // make a new span context
  uint64_t trace_id_high = random_.random();
  uint64_t trace_id = random_.random();
  uint64_t span_id = random_.random();

  SpanContext span_context(
      kDefaultVersion, absl::StrCat(Hex::uint64ToHex(trace_id_high), Hex::uint64ToHex(trace_id)),
      Hex::uint64ToHex(span_id), tracing_decision.traced, "");

  return std::make_unique<Span>(start_time, operation_name, shared_from_this(),
                                std::move(span_context), time_source_, true);
}

// Start a new span with a parent context
Tracing::SpanPtr FluentdTracerImpl::startSpan(SystemTime start_time,
                                              const std::string& operation_name,
                                              const SpanContext& parent_context) {
  // Generate a new span context with new span id based on the parent context.
  SpanContext span_context(kDefaultVersion, parent_context.traceId(),
                           Hex::uint64ToHex(random_.random()), parent_context.sampled(),
                           parent_context.tracestate());
  return std::make_unique<Span>(start_time, operation_name, shared_from_this(),
                                std::move(span_context), time_source_, false);
}

void FluentdTracerImpl::packMessage(MessagePackPacker& packer) {
  packer.pack_array(3); // 1 - tag field, 2 - entries array, 3 - options map.
  packer.pack(tag_);
  packer.pack_array(entries_.size());

  for (auto& entry : entries_) {
    packer.pack_array(2); // 1 - time, 2 - record.
    packer.pack(entry->time_);
    packer.pack_map(entry->map_record_.size());
    for (const auto& pair : entry->map_record_) {
      packer.pack(pair.first);
      packer.pack(pair.second);
    }
  }

  packer.pack(option_);
}

} // namespace Fluentd
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy

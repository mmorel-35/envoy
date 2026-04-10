#include "source/extensions/access_loggers/open_telemetry/http_access_log_impl.h"

#include <memory>
#include <string>
#include <vector>

#include "envoy/config/core/v3/base.pb.h"
#include "envoy/extensions/access_loggers/open_telemetry/v3/logs_service.pb.h"

#include "source/extensions/access_loggers/open_telemetry/otlp_log_utils.h"
#include "source/extensions/access_loggers/open_telemetry/substitution_formatter.h"

namespace Envoy {
namespace Extensions {
namespace AccessLoggers {
namespace OpenTelemetry {

// HttpAccessLoggerImpl and HttpAccessLoggerCacheImpl implementations are in:
// source/extensions/common/opentelemetry/exporters/otlp/http_logs_exporter.cc

HttpAccessLog::ThreadLocalLogger::ThreadLocalLogger(HttpAccessLoggerImpl::SharedPtr logger)
    : logger_(std::move(logger)) {}

HttpAccessLog::HttpAccessLog(
    ::Envoy::AccessLog::FilterPtr&& filter,
    envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config,
    HttpAccessLoggerCacheSharedPtr access_logger_cache,
    Server::Configuration::ServerFactoryContext& server_context,
    const std::vector<Formatter::CommandParserPtr>& commands)
    : Common::ImplBase(std::move(filter)), tls_slot_(server_context.threadLocal().allocateSlot()),
      access_logger_cache_(std::move(access_logger_cache)), http_service_(config.http_service()),
      filter_state_objects_to_log_(getFilterStateObjectsToLog(config)),
      custom_tags_(getCustomTags(config)) {

  // Get or create the headers applicator on the main thread. This is required because
  // DataSourceProvider (used by FILE_CONTENT formatter) allocates TLS slots,
  // which can only happen on the main thread.
  std::shared_ptr<const Http::HttpServiceHeadersApplicator> headers_applicator =
      access_logger_cache_->getOrCreateApplicator(http_service_, server_context);

  tls_slot_->set([this, config, headers_applicator](Event::Dispatcher&) {
    return std::make_shared<ThreadLocalLogger>(
        access_logger_cache_->getOrCreateLogger(config, http_service_, headers_applicator));
  });

  // Packs the body "AnyValue" to a "KeyValueList" only if it's not empty. Otherwise the
  // formatter would fail to parse it.
  if (config.body().value_case() != ::opentelemetry::proto::common::v1::AnyValue::VALUE_NOT_SET) {
    body_formatter_ = std::make_unique<OpenTelemetryFormatter>(packBody(config.body()), commands);
  }
  attributes_formatter_ = std::make_unique<OpenTelemetryFormatter>(config.attributes(), commands);
}

void HttpAccessLog::emitLog(const Formatter::Context& log_context,
                            const StreamInfo::StreamInfo& stream_info) {
  opentelemetry::proto::logs::v1::LogRecord log_entry;
  log_entry.set_time_unix_nano(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                   stream_info.startTime().time_since_epoch())
                                   .count());

  // Unpacks the body "KeyValueList" to "AnyValue".
  if (body_formatter_) {
    const auto formatted_body = unpackBody(body_formatter_->format(log_context, stream_info));
    *log_entry.mutable_body() = formatted_body;
  }
  const auto formatted_attributes = attributes_formatter_->format(log_context, stream_info);
  *log_entry.mutable_attributes() = formatted_attributes.values();

  // Sets trace context (trace_id, span_id) if available.
  const std::string trace_id_hex =
      log_context.activeSpan().has_value() ? log_context.activeSpan()->getTraceId() : "";
  const std::string span_id_hex =
      log_context.activeSpan().has_value() ? log_context.activeSpan()->getSpanId() : "";
  populateTraceContext(log_entry, trace_id_hex, span_id_hex);

  addFilterStateToAttributes(stream_info, filter_state_objects_to_log_, log_entry);
  addCustomTagsToAttributes(custom_tags_, log_context, stream_info, log_entry);

  tls_slot_->getTyped<ThreadLocalLogger>().logger_->log(std::move(log_entry));
}

} // namespace OpenTelemetry
} // namespace AccessLoggers
} // namespace Extensions
} // namespace Envoy

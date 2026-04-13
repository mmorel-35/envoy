#pragma once

#include <memory>
#include <vector>

#include "envoy/access_log/access_log.h"
#include "envoy/extensions/access_loggers/open_telemetry/v3/logs_service.pb.h"
#include "envoy/server/factory_context.h"
#include "envoy/thread_local/thread_local.h"

#include "source/common/tracing/custom_tag_impl.h"
#include "source/extensions/access_loggers/common/access_log_base.h"
#include "source/extensions/access_loggers/open_telemetry/otlp_log_utils.h"
#include "source/extensions/access_loggers/open_telemetry/substitution_formatter.h"
#include "source/extensions/common/opentelemetry/exporters/otlp/http_logs_exporter.h"

namespace Envoy {
namespace Extensions {
namespace AccessLoggers {
namespace OpenTelemetry {

// Backward-compatible aliases pointing to the shared OTLP HTTP logs exporters.
using HttpAccessLoggerImpl =
    ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OtlpHttpLogsExporter;
using HttpAccessLoggerCacheImpl =
    ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OtlpHttpLogsExporterCache;
using HttpAccessLoggerCacheSharedPtr =
    ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OtlpHttpLogsExporterCacheSharedPtr;

/**
 * Access log instance that streams logs over HTTP.
 */
class HttpAccessLog : public Common::ImplBase {
public:
  HttpAccessLog(
      ::Envoy::AccessLog::FilterPtr&& filter,
      envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config,
      HttpAccessLoggerCacheSharedPtr access_logger_cache,
      Server::Configuration::ServerFactoryContext& server_context,
      const std::vector<Formatter::CommandParserPtr>& commands);

private:
  /**
   * Per-thread cached logger.
   */
  struct ThreadLocalLogger : public ThreadLocal::ThreadLocalObject {
    ThreadLocalLogger(HttpAccessLoggerImpl::SharedPtr logger);

    const HttpAccessLoggerImpl::SharedPtr logger_;
  };

  // Common::ImplBase
  void emitLog(const Formatter::Context& context, const StreamInfo::StreamInfo& info) override;

  const ThreadLocal::SlotPtr tls_slot_;
  const HttpAccessLoggerCacheSharedPtr access_logger_cache_;
  const envoy::config::core::v3::HttpService http_service_;
  std::unique_ptr<OpenTelemetryFormatter> body_formatter_;
  std::unique_ptr<OpenTelemetryFormatter> attributes_formatter_;
  const std::vector<std::string> filter_state_objects_to_log_;
  const std::vector<Tracing::CustomTagConstSharedPtr> custom_tags_;
};

using HttpAccessLogPtr = std::unique_ptr<HttpAccessLog>;

} // namespace OpenTelemetry
} // namespace AccessLoggers
} // namespace Extensions
} // namespace Envoy

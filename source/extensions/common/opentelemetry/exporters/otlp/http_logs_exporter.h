#pragma once

#include <memory>
#include <vector>

#include "envoy/config/core/v3/http_service.pb.h"
#include "envoy/event/dispatcher.h"
#include "envoy/extensions/access_loggers/open_telemetry/v3/logs_service.pb.h"
#include "envoy/server/factory_context.h"
#include "envoy/singleton/instance.h"
#include "envoy/thread_local/thread_local.h"
#include "envoy/upstream/cluster_manager.h"

#include "source/common/common/logger.h"
#include "source/common/http/async_client_impl.h"
#include "source/common/http/async_client_utility.h"
#include "source/common/http/http_service_headers.h"
#include "source/common/http/message_impl.h"
#include "source/common/http/utility.h"
#include "source/common/protobuf/protobuf.h"
#include "source/extensions/common/opentelemetry/exporters/otlp/logs_utils.h"

#include "opentelemetry/proto/collector/logs/v1/logs_service.pb.h"
#include "opentelemetry/proto/common/v1/common.pb.h"
#include "opentelemetry/proto/logs/v1/logs.pb.h"
#include "opentelemetry/proto/resource/v1/resource.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

/**
 * HTTP logs exporter that batches and exports OTLP logs over HTTP.
 * Follows the OTLP/HTTP specification.
 */
class OtlpHttpLogsExporter : public Logger::Loggable<Logger::Id::misc>,
                              public Http::AsyncClient::Callbacks {
public:
  OtlpHttpLogsExporter(
      Upstream::ClusterManager& cluster_manager,
      const envoy::config::core::v3::HttpService& http_service,
      std::shared_ptr<const Http::HttpServiceHeadersApplicator> headers_applicator,
      const envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig&
          config,
      Event::Dispatcher& dispatcher, Server::Configuration::ServerFactoryContext& server_context);

  using SharedPtr = std::shared_ptr<OtlpHttpLogsExporter>;

  /**
   * Log a single log entry. Batches entries and flushes periodically.
   */
  void log(opentelemetry::proto::logs::v1::LogRecord&& entry);

  // Http::AsyncClient::Callbacks.
  void onSuccess(const Http::AsyncClient::Request&, Http::ResponseMessagePtr&&) override;
  void onFailure(const Http::AsyncClient::Request&, Http::AsyncClient::FailureReason) override;
  void onBeforeFinalizeUpstreamSpan(Tracing::Span&, const Http::ResponseHeaderMap*) override {}

private:
  void flush();

  Upstream::ClusterManager& cluster_manager_;
  envoy::config::core::v3::HttpService http_service_;
  // Track active HTTP requests to be able to cancel them on destruction.
  Http::AsyncClientRequestTracker active_requests_;
  std::shared_ptr<const Http::HttpServiceHeadersApplicator> headers_applicator_;

  // Message structure: ExportLogsServiceRequest -> ResourceLogs -> ScopeLogs -> LogRecord.
  opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest message_;
  opentelemetry::proto::logs::v1::ScopeLogs* root_;

  // Batching timer.
  Event::TimerPtr flush_timer_;
  const std::chrono::milliseconds buffer_flush_interval_;
  const uint64_t max_buffer_size_bytes_;
  uint64_t approximate_message_size_bytes_ = 0;

  OtlpAccessLogStats stats_;
  uint32_t batched_log_entries_ = 0;
  uint32_t in_flight_log_entries_ = 0;
};

/**
 * Cache for OTLP HTTP logs exporters. Creates one exporter per unique configuration.
 */
class OtlpHttpLogsExporterCache
    : public Singleton::Instance,
      public Logger::Loggable<Logger::Id::misc>,
      public std::enable_shared_from_this<OtlpHttpLogsExporterCache> {
public:
  OtlpHttpLogsExporterCache(Server::Configuration::ServerFactoryContext& server_context);

  OtlpHttpLogsExporter::SharedPtr getOrCreateLogger(
      const envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig&
          config,
      const envoy::config::core::v3::HttpService& http_service,
      std::shared_ptr<const Http::HttpServiceHeadersApplicator> headers_applicator);

  std::shared_ptr<const Http::HttpServiceHeadersApplicator>
  getOrCreateApplicator(const envoy::config::core::v3::HttpService& http_service,
                        Server::Configuration::ServerFactoryContext& server_context);

private:
  struct ThreadLocalCache : public ThreadLocal::ThreadLocalObject {
    ThreadLocalCache(Event::Dispatcher& dispatcher) : dispatcher_(dispatcher) {}
    Event::Dispatcher& dispatcher_;
    absl::flat_hash_map<std::size_t, OtlpHttpLogsExporter::SharedPtr> access_loggers_;
  };

  ThreadLocal::SlotPtr tls_slot_;
  Server::Configuration::ServerFactoryContext& server_context_;

  // Cache of headers applicators, keyed by HttpService config hash. Protected by
  // applicator_mutex_ because the custom deleter on the shared_ptr may run on any thread.
  absl::Mutex applicator_mutex_;
  absl::flat_hash_map<std::size_t, std::weak_ptr<const Http::HttpServiceHeadersApplicator>>
      applicators_ ABSL_GUARDED_BY(applicator_mutex_);
};

using OtlpHttpLogsExporterCacheSharedPtr = std::shared_ptr<OtlpHttpLogsExporterCache>;

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

#include "source/extensions/common/opentelemetry/exporters/otlp/http_logs_exporter.h"

#include <memory>
#include <string>
#include <vector>

#include "envoy/config/core/v3/base.pb.h"
#include "envoy/data/accesslog/v3/accesslog.pb.h"
#include "envoy/extensions/access_loggers/open_telemetry/v3/logs_service.pb.h"

#include "source/common/common/enum_to_int.h"
#include "source/common/common/logger.h"
#include "source/common/config/utility.h"
#include "source/common/http/headers.h"
#include "source/common/protobuf/utility.h"
#include "source/extensions/common/opentelemetry/exporters/otlp/logs_utils.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

OtlpHttpLogsExporter::OtlpHttpLogsExporter(
    Upstream::ClusterManager& cluster_manager,
    const envoy::config::core::v3::HttpService& http_service,
    std::shared_ptr<const Http::HttpServiceHeadersApplicator> headers_applicator,
    const envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig&
        config,
    Event::Dispatcher& dispatcher, Server::Configuration::ServerFactoryContext& server_context)
    : cluster_manager_(cluster_manager), http_service_(http_service),
      headers_applicator_(std::move(headers_applicator)),
      buffer_flush_interval_(getBufferFlushInterval(config)),
      max_buffer_size_bytes_(getBufferSizeBytes(config)),
      stats_({ALL_OTLP_ACCESS_LOG_STATS(
          POOL_COUNTER_PREFIX(server_context.serverScope(),
                              absl::StrCat(OtlpAccessLogStatsPrefix, config.stat_prefix())))}) {

  root_ = initOtlpMessageRoot(message_, config, server_context.localInfo());

  // Sets up the flush timer.
  flush_timer_ = dispatcher.createTimer([this]() {
    flush();
    flush_timer_->enableTimer(buffer_flush_interval_);
  });
  flush_timer_->enableTimer(buffer_flush_interval_);
}

void OtlpHttpLogsExporter::log(opentelemetry::proto::logs::v1::LogRecord&& entry) {
  approximate_message_size_bytes_ += entry.ByteSizeLong();
  batched_log_entries_++;
  root_->mutable_log_records()->Add(std::move(entry));

  if (approximate_message_size_bytes_ >= max_buffer_size_bytes_) {
    flush();
  }
}

void OtlpHttpLogsExporter::flush() {
  if (root_->log_records().empty()) {
    return;
  }

  std::string request_body;
  const auto ok = message_.SerializeToString(&request_body);
  if (!ok) {
    ENVOY_LOG(warn, "Error while serializing the binary proto ExportLogsServiceRequest.");
    root_->clear_log_records();
    approximate_message_size_bytes_ = 0;
    return;
  }

  const auto thread_local_cluster =
      cluster_manager_.getThreadLocalCluster(http_service_.http_uri().cluster());
  if (thread_local_cluster == nullptr) {
    ENVOY_LOG(error, "OTLP HTTP logs exporter failed: [cluster = {}] is not configured",
              http_service_.http_uri().cluster());
    root_->clear_log_records();
    approximate_message_size_bytes_ = 0;
    return;
  }

  Http::RequestMessagePtr message = Http::Utility::prepareHeaders(http_service_.http_uri());

  // The request follows the OTLP HTTP specification:
  // https://github.com/open-telemetry/opentelemetry-proto/blob/v1.9.0/docs/specification.md#otlphttp.
  message->headers().setReferenceMethod(Http::Headers::get().MethodValues.Post);
  message->headers().setReferenceContentType(Http::Headers::get().ContentTypeValues.Protobuf);

  // User-Agent header follows the OTLP specification.
  message->headers().setReferenceUserAgent(GetUserAgent());

  // Adds all custom headers to the request.
  headers_applicator_->apply(message->headers());

  message->body().add(request_body);

  const auto options =
      Http::AsyncClient::RequestOptions()
          .setTimeout(std::chrono::milliseconds(
              DurationUtil::durationToMilliseconds(http_service_.http_uri().timeout())))
          .setDiscardResponseBody(true);

  Http::AsyncClient::Request* in_flight_request =
      thread_local_cluster->httpAsyncClient().send(std::move(message), *this, options);

  if (in_flight_request != nullptr) {
    active_requests_.add(*in_flight_request);
    in_flight_log_entries_ = batched_log_entries_;
  } else {
    stats_.logs_dropped_.add(batched_log_entries_);
  }

  root_->clear_log_records();
  approximate_message_size_bytes_ = 0;
  batched_log_entries_ = 0;
}

void OtlpHttpLogsExporter::onSuccess(const Http::AsyncClient::Request& request,
                                      Http::ResponseMessagePtr&& http_response) {
  active_requests_.remove(request);
  const auto response_code = Http::Utility::getResponseStatus(http_response->headers());
  if (response_code == enumToInt(Http::Code::OK)) {
    stats_.logs_written_.add(in_flight_log_entries_);
  } else {
    ENVOY_LOG(error,
              "OTLP HTTP logs exporter received a non-success status code: {} while "
              "exporting the OTLP message",
              response_code);
    stats_.logs_dropped_.add(in_flight_log_entries_);
  }
  in_flight_log_entries_ = 0;
}

void OtlpHttpLogsExporter::onFailure(const Http::AsyncClient::Request& request,
                                      Http::AsyncClient::FailureReason reason) {
  active_requests_.remove(request);
  ENVOY_LOG(warn, "OTLP HTTP logs export request failed. Failure reason: {}", enumToInt(reason));
  stats_.logs_dropped_.add(in_flight_log_entries_);
  in_flight_log_entries_ = 0;
}

OtlpHttpLogsExporterCache::OtlpHttpLogsExporterCache(
    Server::Configuration::ServerFactoryContext& server_context)
    : tls_slot_(server_context.threadLocal().allocateSlot()), server_context_(server_context) {
  tls_slot_->set(
      [](Event::Dispatcher& dispatcher) { return std::make_shared<ThreadLocalCache>(dispatcher); });
}

OtlpHttpLogsExporter::SharedPtr OtlpHttpLogsExporterCache::getOrCreateLogger(
    const envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig&
        config,
    const envoy::config::core::v3::HttpService& http_service,
    std::shared_ptr<const Http::HttpServiceHeadersApplicator> headers_applicator) {
  auto& cache = tls_slot_->getTyped<ThreadLocalCache>();
  const std::size_t config_hash = MessageUtil::hash(config) ^ MessageUtil::hash(http_service);

  const auto it = cache.access_loggers_.find(config_hash);
  if (it != cache.access_loggers_.end()) {
    return it->second;
  }

  auto logger = std::make_shared<OtlpHttpLogsExporter>(server_context_.clusterManager(),
                                                        http_service, std::move(headers_applicator),
                                                        config, cache.dispatcher_, server_context_);
  cache.access_loggers_.emplace(config_hash, logger);
  return logger;
}

std::shared_ptr<const Http::HttpServiceHeadersApplicator>
OtlpHttpLogsExporterCache::getOrCreateApplicator(
    const envoy::config::core::v3::HttpService& http_service,
    Server::Configuration::ServerFactoryContext& server_context) {
  ASSERT_IS_MAIN_OR_TEST_THREAD();
  const std::size_t config_hash = MessageUtil::hash(http_service);

  absl::MutexLock lock(&applicator_mutex_);

  const auto it = applicators_.find(config_hash);
  if (it != applicators_.end()) {
    auto existing = it->second.lock();
    if (existing) {
      return existing;
    }
  }

  // If this object cannot be created, it is critical that the `shared_ptr` custom deleter
  // below is not run, because it would deadlock as the mutex is already held.
  std::unique_ptr<Http::HttpServiceHeadersApplicator> headers_applicator =
      Http::HttpServiceHeadersApplicator::createOrThrow(http_service, server_context);

  // Capture shared_from_this() in the deleter so the mutex and map remain alive.
  std::shared_ptr<OtlpHttpLogsExporterCache> self = shared_from_this();
  std::shared_ptr<const Http::HttpServiceHeadersApplicator> applicator(
      headers_applicator.release(),
      [self, config_hash](const Http::HttpServiceHeadersApplicator* ptr) {
        {
          absl::MutexLock lock(&self->applicator_mutex_);
          const auto it = self->applicators_.find(config_hash);
          // Check for expired in case a new entry was added at nearly the same time because the
          // check for an existing entry failed to `lock()`.
          if (it != self->applicators_.end() && it->second.expired()) {
            self->applicators_.erase(it);
          }
        }
        delete ptr;
      });
  applicators_.insert_or_assign(config_hash, applicator);
  return applicator;
}

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

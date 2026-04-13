#include "source/extensions/common/opentelemetry/exporters/otlp/grpc_logs_exporter.h"

#include "envoy/extensions/access_loggers/grpc/v3/als.pb.h"
#include "envoy/extensions/access_loggers/open_telemetry/v3/logs_service.pb.h"
#include "envoy/grpc/async_client_manager.h"
#include "envoy/local_info/local_info.h"

#include "source/common/config/utility.h"
#include "source/common/grpc/typed_async_client.h"
#include "source/common/protobuf/utility.h"
#include "source/extensions/access_loggers/common/grpc_access_logger_clients.h"
#include "source/extensions/common/opentelemetry/exporters/otlp/logs_utils.h"
#include "source/extensions/common/opentelemetry/sdk/logs/constants.h"

#include "opentelemetry/proto/collector/logs/v1/logs_service.pb.h"
#include "opentelemetry/proto/common/v1/common.pb.h"
#include "opentelemetry/proto/logs/v1/logs.pb.h"
#include "opentelemetry/proto/resource/v1/resource.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

namespace {
using opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest;
using opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse;
} // namespace

OtlpGrpcLogsExporter::OtlpGrpcLogsExporter(
    const Grpc::RawAsyncClientSharedPtr& client,
    const envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig&
        config,
    Event::Dispatcher& dispatcher, const LocalInfo::LocalInfo& local_info, Stats::Scope& scope)
    : GrpcAccessLogger(
          config.common_config(), dispatcher, scope, std::nullopt,
          std::make_unique<AccessLoggers::Common::UnaryGrpcAccessLogClient<ExportLogsServiceRequest,
                                                                           ExportLogsServiceResponse>>(
              client,
              *Protobuf::DescriptorPool::generated_pool()->FindMethodByName(
                  std::string(Envoy::Extensions::OpenTelemetry::Sdk::Logs::Constants::
                                  kLogsServiceExportMethod)),
              AccessLoggers::Common::GrpcCommon::optionalRetryPolicy(config.common_config()),
              genOTelCallbacksFactory())),
      stats_({ALL_GRPC_ACCESS_LOGGER_STATS(
          POOL_COUNTER_PREFIX(scope, absl::StrCat(OtlpAccessLogStatsPrefix, config.stat_prefix()))
              )}) {
  root_ = initOtlpMessageRoot(message_, config, local_info);
}

std::function<OtlpGrpcLogsExporter::OTelLogRequestCallbacks&()>
OtlpGrpcLogsExporter::genOTelCallbacksFactory() {
  return [this]() -> OTelLogRequestCallbacks& {
    auto callback = std::make_unique<OTelLogRequestCallbacks>(
        this->stats_, this->batched_log_entries_, [this](OTelLogRequestCallbacks* p) {
          if (this->callbacks_.contains(p)) {
            this->callbacks_.erase(p);
          }
        });
    OTelLogRequestCallbacks* ptr = callback.get();
    this->batched_log_entries_ = 0;
    this->callbacks_.emplace(ptr, std::move(callback));
    return *ptr;
  };
}

void OtlpGrpcLogsExporter::addEntry(opentelemetry::proto::logs::v1::LogRecord&& entry) {
  batched_log_entries_++;
  root_->mutable_log_records()->Add(std::move(entry));
}

bool OtlpGrpcLogsExporter::isEmpty() { return root_->log_records().empty(); }

// The message is already initialized in the c'tor, and only the logs are cleared.
void OtlpGrpcLogsExporter::initMessage() {}

void OtlpGrpcLogsExporter::clearMessage() { root_->clear_log_records(); }

OtlpGrpcLogsExporterCache::OtlpGrpcLogsExporterCache(
    Grpc::AsyncClientManager& async_client_manager, Stats::Scope& scope,
    ThreadLocal::SlotAllocator& tls, const LocalInfo::LocalInfo& local_info)
    : GrpcAccessLoggerCache(async_client_manager, scope, tls), local_info_(local_info) {}

OtlpGrpcLogsExporter::SharedPtr OtlpGrpcLogsExporterCache::createLogger(
    const envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig&
        config,
    Event::Dispatcher& dispatcher) {
  // We pass skip_cluster_check=true to factoryForGrpcService in order to avoid throwing
  // exceptions in worker threads. Call sites of this getOrCreateLogger must check the cluster
  // availability via ClusterManager::checkActiveStaticCluster beforehand, and throw exceptions in
  // the main thread if necessary to ensure it does not throw here.
  auto factory_or_error =
      async_client_manager_.factoryForGrpcService(getGrpcService(config), scope_, true);
  THROW_IF_NOT_OK_REF(factory_or_error.status());
  auto client = THROW_OR_RETURN_VALUE(factory_or_error.value()->createUncachedRawAsyncClient(),
                                      Grpc::RawAsyncClientPtr);
  return std::make_shared<OtlpGrpcLogsExporter>(std::move(client), config, dispatcher, local_info_,
                                                 scope_);
}

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

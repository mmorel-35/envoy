#include <chrono>
#include <memory>

#include "envoy/config/core/v3/grpc_service.pb.h"
#include "envoy/extensions/access_loggers/grpc/v3/als.pb.h"

#include "source/common/buffer/zero_copy_input_stream_impl.h"
#include "source/common/grpc/common.h"
#include "source/common/http/http_service_headers.h"
#include "source/common/protobuf/protobuf.h"
#include "source/extensions/common/opentelemetry/exporters/otlp/grpc_logs_exporter.h"
#include "source/extensions/common/opentelemetry/exporters/otlp/http_logs_exporter.h"

#include "test/mocks/event/mocks.h"
#include "test/mocks/grpc/mocks.h"
#include "test/mocks/local_info/mocks.h"
#include "test/mocks/server/factory_context.h"
#include "test/mocks/server/server_factory_context.h"
#include "test/mocks/stats/mocks.h"
#include "test/mocks/thread_local/mocks.h"
#include "test/mocks/upstream/cluster_manager.h"
#include "test/test_common/utility.h"

#include "absl/strings/match.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "opentelemetry/proto/collector/logs/v1/logs_service.pb.h"
#include "opentelemetry/proto/common/v1/common.pb.h"
#include "opentelemetry/proto/logs/v1/logs.pb.h"
#include "opentelemetry/proto/resource/v1/resource.pb.h"

using testing::_;
using testing::Invoke;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {
namespace {

constexpr std::chrono::milliseconds FlushInterval(10);
constexpr int BUFFER_SIZE_BYTES = 0;
const std::string ZONE_NAME = "zone_name";
const std::string CLUSTER_NAME = "cluster_name";
const std::string NODE_NAME = "node_name";

// --------------------------------------------------------------------------
// Helper to mock gRPC send and intercept callbacks.
// --------------------------------------------------------------------------
class OtlpGrpcLogsExporterTestHelper {
public:
  OtlpGrpcLogsExporterTestHelper(LocalInfo::MockLocalInfo& local_info,
                                  Grpc::MockAsyncClient* async_client, bool expect_call = true)
      : async_client_(async_client) {
    if (expect_call) {
      EXPECT_CALL(local_info, zoneName()).WillOnce(ReturnRef(ZONE_NAME));
      EXPECT_CALL(local_info, clusterName()).WillOnce(ReturnRef(CLUSTER_NAME));
      EXPECT_CALL(local_info, nodeName()).WillOnce(ReturnRef(NODE_NAME));
    }
  }

  void expectSentMessage(
      const std::string& expected_message_yaml,
      Grpc::Status::GrpcStatus response_status = Grpc::Status::WellKnownGrpcStatus::Ok,
      uint32_t rejected_log_records = 0) {
    opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest expected_message;
    TestUtility::loadFromYaml(expected_message_yaml, expected_message);
    EXPECT_CALL(*async_client_, sendRaw(_, _, _, _, _, _))
        .WillOnce(Invoke([expected_message, response_status, rejected_log_records](
                             absl::string_view, absl::string_view, Buffer::InstancePtr&& request,
                             Grpc::RawAsyncRequestCallbacks& callback, Tracing::Span& span,
                             const Http::AsyncClient::RequestOptions&) {
          opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest message;

          Buffer::ZeroCopyInputStreamImpl request_stream(std::move(request));
          EXPECT_TRUE(message.ParseFromZeroCopyStream(&request_stream));
          EXPECT_EQ(message.DebugString(), expected_message.DebugString());
          if (response_status != Grpc::Status::WellKnownGrpcStatus::Ok) {
            callback.onFailure(response_status, "err", span);
          } else {
            opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse resp;
            resp.mutable_partial_success()->set_rejected_log_records(rejected_log_records);
            callback.onSuccessRaw(Grpc::Common::serializeMessage(resp), span);
          }
          return nullptr;
        }));
  }

private:
  Grpc::MockAsyncClient* async_client_;
};

// --------------------------------------------------------------------------
// OtlpGrpcLogsExporter unit tests
// --------------------------------------------------------------------------
class OtlpGrpcLogsExporterTest : public testing::Test {
public:
  OtlpGrpcLogsExporterTest()
      : async_client_(new Grpc::MockAsyncClient), timer_(new Event::MockTimer(&dispatcher_)),
        helper_(local_info_, async_client_, true) {
    EXPECT_CALL(*timer_, enableTimer(_, _));
    *config_.mutable_common_config()->mutable_log_name() = "test_log_name";
    config_.mutable_common_config()->mutable_buffer_size_bytes()->set_value(BUFFER_SIZE_BYTES);
    config_.mutable_common_config()->mutable_buffer_flush_interval()->set_nanos(
        std::chrono::duration_cast<std::chrono::nanoseconds>(FlushInterval).count());
  }

  Grpc::MockAsyncClient* async_client_;
  NiceMock<Stats::MockIsolatedStatsStore> stats_store_;
  LocalInfo::MockLocalInfo local_info_;
  Event::MockDispatcher dispatcher_;
  Event::MockTimer* timer_;
  std::unique_ptr<OtlpGrpcLogsExporter> exporter_;
  OtlpGrpcLogsExporterTestHelper helper_;
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config_;

  void createExporter() {
    exporter_ = std::make_unique<OtlpGrpcLogsExporter>(Grpc::RawAsyncClientPtr{async_client_},
                                                        config_, dispatcher_, local_info_,
                                                        *stats_store_.rootScope());
  }
};

TEST_F(OtlpGrpcLogsExporterTest, Log) {
  createExporter();
  helper_.expectSentMessage(R"EOF(
  resource_logs:
    resource:
      attributes:
        - key: "log_name"
          value:
            string_value: "test_log_name"
        - key: "zone_name"
          value:
            string_value: "zone_name"
        - key: "cluster_name"
          value:
            string_value: "cluster_name"
        - key: "node_name"
          value:
            string_value: "node_name"
    scope_logs:
      - log_records:
          - severity_text: "test-severity-text"
  )EOF");
  opentelemetry::proto::logs::v1::LogRecord entry;
  entry.set_severity_text("test-severity-text");
  exporter_->log(opentelemetry::proto::logs::v1::LogRecord(entry));
}

TEST_F(OtlpGrpcLogsExporterTest, LogWithStats) {
  createExporter();

  helper_.expectSentMessage(R"EOF(
  resource_logs:
    scope_logs:
      - log_records:
          - severity_text: "test-severity-text"
  )EOF",
                             Grpc::Status::WellKnownGrpcStatus::Ok, 0);

  opentelemetry::proto::logs::v1::LogRecord entry;
  entry.set_severity_text("test-severity-text");
  exporter_->log(opentelemetry::proto::logs::v1::LogRecord(entry));

  EXPECT_EQ(1, stats_store_
                   .findCounterByString("access_logs.open_telemetry_access_log.logs_written")
                   .value()
                   .get()
                   .value());
}

TEST_F(OtlpGrpcLogsExporterTest, PartialSuccessStats) {
  createExporter();

  helper_.expectSentMessage(R"EOF(
  resource_logs:
    scope_logs:
      - log_records:
          - severity_text: "one"
  )EOF",
                             Grpc::Status::WellKnownGrpcStatus::Ok, 1);

  opentelemetry::proto::logs::v1::LogRecord entry;
  entry.set_severity_text("one");
  exporter_->log(opentelemetry::proto::logs::v1::LogRecord(entry));

  EXPECT_EQ(0, stats_store_
                   .findCounterByString("access_logs.open_telemetry_access_log.logs_written")
                   .value()
                   .get()
                   .value());
  EXPECT_EQ(1, stats_store_
                   .findCounterByString("access_logs.open_telemetry_access_log.logs_dropped")
                   .value()
                   .get()
                   .value());
}

TEST_F(OtlpGrpcLogsExporterTest, GrpcFailureStats) {
  createExporter();

  helper_.expectSentMessage(R"EOF(
  resource_logs:
    scope_logs:
      - log_records:
          - severity_text: "x"
  )EOF",
                             Grpc::Status::WellKnownGrpcStatus::Internal);

  opentelemetry::proto::logs::v1::LogRecord entry;
  entry.set_severity_text("x");
  exporter_->log(opentelemetry::proto::logs::v1::LogRecord(entry));

  EXPECT_EQ(1, stats_store_
                   .findCounterByString("access_logs.open_telemetry_access_log.logs_dropped")
                   .value()
                   .get()
                   .value());
}

TEST_F(OtlpGrpcLogsExporterTest, StatsWithCustomPrefix) {
  config_.set_stat_prefix("my_prefix");
  createExporter();

  helper_.expectSentMessage(R"EOF(
  resource_logs:
    scope_logs:
      - log_records:
          - severity_text: "prefixed"
  )EOF");

  opentelemetry::proto::logs::v1::LogRecord entry;
  entry.set_severity_text("prefixed");
  exporter_->log(opentelemetry::proto::logs::v1::LogRecord(entry));

  EXPECT_EQ(1,
            stats_store_
                .findCounterByString("access_logs.open_telemetry_access_log.my_prefix.logs_written")
                .value()
                .get()
                .value());
}

// --------------------------------------------------------------------------
// OtlpGrpcLogsExporterCache tests
// --------------------------------------------------------------------------

class OtlpGrpcLogsExporterCacheTest : public testing::Test {
public:
  OtlpGrpcLogsExporterCacheTest()
      : async_client_(new Grpc::MockAsyncClient), factory_(new Grpc::MockAsyncClientFactory),
        exporter_cache_(async_client_manager_, scope_, tls_, local_info_),
        helper_(local_info_, async_client_, true) {
    EXPECT_CALL(async_client_manager_, factoryForGrpcService(_, _, true))
        .WillOnce(Invoke([this](const envoy::config::core::v3::GrpcService&, Stats::Scope&, bool) {
          EXPECT_CALL(*factory_, createUncachedRawAsyncClient()).WillOnce(Invoke([this] {
            return Grpc::RawAsyncClientPtr{async_client_};
          }));
          return Grpc::AsyncClientFactoryPtr{factory_};
        }));
  }

  Grpc::MockAsyncClient* async_client_;
  Grpc::MockAsyncClientFactory* factory_;
  Grpc::MockAsyncClientManager async_client_manager_;
  LocalInfo::MockLocalInfo local_info_;
  NiceMock<Stats::MockIsolatedStatsStore> stats_store_;
  Stats::Scope& scope_{*stats_store_.rootScope()};
  NiceMock<ThreadLocal::MockInstance> tls_;
  OtlpGrpcLogsExporterCache exporter_cache_;
  OtlpGrpcLogsExporterTestHelper helper_;
};

TEST_F(OtlpGrpcLogsExporterCacheTest, ExporterCreation) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  *config.mutable_common_config()->mutable_log_name() = "test_log_name";
  config.mutable_common_config()->set_transport_api_version(
      envoy::config::core::v3::ApiVersion::V3);
  config.mutable_common_config()->mutable_buffer_size_bytes()->set_value(BUFFER_SIZE_BYTES);

  OtlpGrpcLogsExporterSharedPtr logger =
      exporter_cache_.getOrCreateLogger(config, AccessLoggers::Common::GrpcAccessLoggerType::HTTP);
  helper_.expectSentMessage(R"EOF(
  resource_logs:
    resource:
      attributes:
        - key: "log_name"
          value:
            string_value: "test_log_name"
        - key: "zone_name"
          value:
            string_value: "zone_name"
        - key: "cluster_name"
          value:
            string_value: "cluster_name"
        - key: "node_name"
          value:
            string_value: "node_name"
    scope_logs:
      - log_records:
          - severity_text: "cache-test"
  )EOF");
  opentelemetry::proto::logs::v1::LogRecord entry;
  entry.set_severity_text("cache-test");
  logger->log(opentelemetry::proto::logs::v1::LogRecord(entry));
  EXPECT_EQ(1,
            stats_store_
                .findCounterByString("access_logs.open_telemetry_access_log.logs_written")
                .value()
                .get()
                .value());
}

// --------------------------------------------------------------------------
// OtlpHttpLogsExporter tests
// --------------------------------------------------------------------------

class OtlpHttpLogsExporterTest : public testing::Test {
public:
  OtlpHttpLogsExporterTest() : timer_(new Event::MockTimer(&dispatcher_)) {
    EXPECT_CALL(*timer_, enableTimer(_, _)).Times(testing::AnyNumber());
  }

  void setup(envoy::config::core::v3::HttpService http_service) {
    envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
    setupWithConfig(http_service, config);
  }

  void setupWithConfig(
      envoy::config::core::v3::HttpService http_service,
      envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config) {
    cluster_manager_.thread_local_cluster_.cluster_.info_->name_ = "my_o11y_backend";
    cluster_manager_.initializeThreadLocalClusters({"my_o11y_backend"});
    ON_CALL(cluster_manager_.thread_local_cluster_, httpAsyncClient())
        .WillByDefault(ReturnRef(cluster_manager_.thread_local_cluster_.async_client_));
    cluster_manager_.initializeClusters({"my_o11y_backend"}, {});

    ON_CALL(factory_context_.server_factory_context_.local_info_, zoneName())
        .WillByDefault(ReturnRef(ZONE_NAME));
    ON_CALL(factory_context_.server_factory_context_.local_info_, clusterName())
        .WillByDefault(ReturnRef(CLUSTER_NAME));
    ON_CALL(factory_context_.server_factory_context_.local_info_, nodeName())
        .WillByDefault(ReturnRef(NODE_NAME));

    auto headers_applicator = Http::HttpServiceHeadersApplicator::createOrThrow(
        http_service, factory_context_.server_factory_context_);
    http_exporter_ = std::make_unique<OtlpHttpLogsExporter>(
        cluster_manager_, http_service, std::move(headers_applicator), config, dispatcher_,
        factory_context_.server_factory_context_);
  }

protected:
  NiceMock<Upstream::MockClusterManager> cluster_manager_;
  NiceMock<Event::MockDispatcher> dispatcher_;
  Event::MockTimer* timer_;
  NiceMock<Server::Configuration::MockFactoryContext> factory_context_;
  std::unique_ptr<OtlpHttpLogsExporter> http_exporter_;
};

TEST_F(OtlpHttpLogsExporterTest, ExportLogWithCustomHeaders) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/otlp/v1/logs"
    cluster: "my_o11y_backend"
    timeout: 0.250s
  request_headers_to_add:
  - header:
      key: "Authorization"
      value: "Bearer test-token"
  )EOF";

  envoy::config::core::v3::HttpService http_service;
  TestUtility::loadFromYaml(yaml_string, http_service);
  setup(http_service);

  Http::MockAsyncClientRequest request(&cluster_manager_.thread_local_cluster_.async_client_);
  Http::AsyncClient::Callbacks* callback;

  EXPECT_CALL(cluster_manager_.thread_local_cluster_.async_client_,
              send_(_, _,
                    Http::AsyncClient::RequestOptions()
                        .setTimeout(std::chrono::milliseconds(250))
                        .setDiscardResponseBody(true)))
      .WillOnce(
          Invoke([&](Http::RequestMessagePtr& message, Http::AsyncClient::Callbacks& callbacks,
                     const Http::AsyncClient::RequestOptions&) -> Http::AsyncClient::Request* {
            callback = &callbacks;

            EXPECT_EQ(Http::Headers::get().MethodValues.Post, message->headers().getMethodValue());
            EXPECT_EQ(Http::Headers::get().ContentTypeValues.Protobuf,
                      message->headers().getContentTypeValue());
            EXPECT_EQ("/otlp/v1/logs", message->headers().getPathValue());
            EXPECT_EQ("some-o11y.com", message->headers().getHostValue());
            EXPECT_TRUE(absl::StartsWith(message->headers().getUserAgentValue(),
                                         "OTel-OTLP-Exporter-Envoy/"));
            EXPECT_EQ("Bearer test-token",
                      message->headers()
                          .get(Http::LowerCaseString("authorization"))[0]
                          ->value()
                          .getStringView());

            return &request;
          }));

  opentelemetry::proto::logs::v1::LogRecord log_record;
  log_record.set_severity_number(opentelemetry::proto::logs::v1::SEVERITY_NUMBER_INFO);
  log_record.mutable_body()->set_string_value("test log entry");
  http_exporter_->log(std::move(log_record));

  Http::ResponseMessagePtr msg(new Http::ResponseMessageImpl(
      Http::ResponseHeaderMapPtr{new Http::TestResponseHeaderMapImpl{{":status", "200"}}}));

  Tracing::NullSpan null_span;
  callback->onBeforeFinalizeUpstreamSpan(null_span, nullptr);
  callback->onSuccess(request, std::move(msg));
}

TEST_F(OtlpHttpLogsExporterTest, ExportWithoutCluster) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/otlp/v1/logs"
    cluster: "my_o11y_backend"
    timeout: 10s
  )EOF";

  envoy::config::core::v3::HttpService http_service;
  TestUtility::loadFromYaml(yaml_string, http_service);
  setup(http_service);

  ON_CALL(cluster_manager_, getThreadLocalCluster(absl::string_view("my_o11y_backend")))
      .WillByDefault(Return(nullptr));

  opentelemetry::proto::logs::v1::LogRecord log_record;
  log_record.mutable_body()->set_string_value("dropped log");
  http_exporter_->log(std::move(log_record));
}

TEST_F(OtlpHttpLogsExporterTest, NonSuccessStatusCode) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/otlp/v1/logs"
    cluster: "my_o11y_backend"
    timeout: 0.250s
  )EOF";

  envoy::config::core::v3::HttpService http_service;
  TestUtility::loadFromYaml(yaml_string, http_service);
  setup(http_service);

  Http::MockAsyncClientRequest request(&cluster_manager_.thread_local_cluster_.async_client_);
  Http::AsyncClient::Callbacks* callback;

  EXPECT_CALL(cluster_manager_.thread_local_cluster_.async_client_, send_(_, _, _))
      .WillOnce(
          Invoke([&](Http::RequestMessagePtr&, Http::AsyncClient::Callbacks& callbacks,
                     const Http::AsyncClient::RequestOptions&) -> Http::AsyncClient::Request* {
            callback = &callbacks;
            return &request;
          }));

  opentelemetry::proto::logs::v1::LogRecord log_record;
  log_record.mutable_body()->set_string_value("test");
  http_exporter_->log(std::move(log_record));

  Http::ResponseMessagePtr msg(new Http::ResponseMessageImpl(
      Http::ResponseHeaderMapPtr{new Http::TestResponseHeaderMapImpl{{":status", "503"}}}));
  callback->onSuccess(request, std::move(msg));
}

TEST_F(OtlpHttpLogsExporterTest, HttpFailure) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/otlp/v1/logs"
    cluster: "my_o11y_backend"
    timeout: 0.250s
  )EOF";

  envoy::config::core::v3::HttpService http_service;
  TestUtility::loadFromYaml(yaml_string, http_service);
  setup(http_service);

  Http::MockAsyncClientRequest request(&cluster_manager_.thread_local_cluster_.async_client_);
  Http::AsyncClient::Callbacks* callback;

  EXPECT_CALL(cluster_manager_.thread_local_cluster_.async_client_, send_(_, _, _))
      .WillOnce(
          Invoke([&](Http::RequestMessagePtr&, Http::AsyncClient::Callbacks& callbacks,
                     const Http::AsyncClient::RequestOptions&) -> Http::AsyncClient::Request* {
            callback = &callbacks;
            return &request;
          }));

  opentelemetry::proto::logs::v1::LogRecord log_record;
  log_record.mutable_body()->set_string_value("test");
  http_exporter_->log(std::move(log_record));
  callback->onFailure(request, Http::AsyncClient::FailureReason::Reset);
}

TEST_F(OtlpHttpLogsExporterTest, SendNullptr) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/otlp/v1/logs"
    cluster: "my_o11y_backend"
    timeout: 0.250s
  )EOF";

  envoy::config::core::v3::HttpService http_service;
  TestUtility::loadFromYaml(yaml_string, http_service);
  setup(http_service);

  EXPECT_CALL(cluster_manager_.thread_local_cluster_.async_client_, send_(_, _, _))
      .WillOnce(Return(nullptr));

  opentelemetry::proto::logs::v1::LogRecord log_record;
  log_record.mutable_body()->set_string_value("dropped");
  http_exporter_->log(std::move(log_record));
}

TEST_F(OtlpHttpLogsExporterTest, BufferOverflowTriggersFlush) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/otlp/v1/logs"
    cluster: "my_o11y_backend"
    timeout: 0.250s
  )EOF";

  envoy::config::core::v3::HttpService http_service;
  TestUtility::loadFromYaml(yaml_string, http_service);

  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  config.mutable_buffer_size_bytes()->set_value(1);
  setupWithConfig(http_service, config);

  Http::MockAsyncClientRequest request(&cluster_manager_.thread_local_cluster_.async_client_);
  Http::AsyncClient::Callbacks* callback;

  EXPECT_CALL(cluster_manager_.thread_local_cluster_.async_client_, send_(_, _, _))
      .WillOnce(
          Invoke([&](Http::RequestMessagePtr&, Http::AsyncClient::Callbacks& callbacks,
                     const Http::AsyncClient::RequestOptions&) -> Http::AsyncClient::Request* {
            callback = &callbacks;
            return &request;
          }));

  opentelemetry::proto::logs::v1::LogRecord log_record;
  log_record.mutable_body()->set_string_value("exceeds buffer limit");
  http_exporter_->log(std::move(log_record));

  Http::ResponseMessagePtr msg(new Http::ResponseMessageImpl(
      Http::ResponseHeaderMapPtr{new Http::TestResponseHeaderMapImpl{{":status", "200"}}}));
  callback->onSuccess(request, std::move(msg));
}

// --------------------------------------------------------------------------
// OtlpHttpLogsExporterCache tests
// --------------------------------------------------------------------------

TEST(OtlpHttpLogsExporterCacheTest, CacheHitReturnsSameExporter) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/otlp/v1/logs"
    cluster: "my_o11y_backend"
    timeout: 0.250s
  )EOF";

  envoy::config::core::v3::HttpService http_service;
  TestUtility::loadFromYaml(yaml_string, http_service);

  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  config.set_log_name("test_log");

  NiceMock<Server::Configuration::MockFactoryContext> factory_context;

  factory_context.server_factory_context_.cluster_manager_.thread_local_cluster_.cluster_.info_
      ->name_ = "my_o11y_backend";
  factory_context.server_factory_context_.cluster_manager_.initializeThreadLocalClusters(
      {"my_o11y_backend"});
  factory_context.server_factory_context_.cluster_manager_.initializeClusters({"my_o11y_backend"},
                                                                              {});

  ON_CALL(factory_context.server_factory_context_.local_info_, zoneName())
      .WillByDefault(ReturnRef(ZONE_NAME));
  ON_CALL(factory_context.server_factory_context_.local_info_, clusterName())
      .WillByDefault(ReturnRef(CLUSTER_NAME));
  ON_CALL(factory_context.server_factory_context_.local_info_, nodeName())
      .WillByDefault(ReturnRef(NODE_NAME));

  auto cache =
      std::make_shared<OtlpHttpLogsExporterCache>(factory_context.server_factory_context_);

  std::shared_ptr<const Http::HttpServiceHeadersApplicator> headers_applicator =
      Http::HttpServiceHeadersApplicator::createOrThrow(http_service,
                                                        factory_context.server_factory_context_);

  auto exporter1 = cache->getOrCreateLogger(config, http_service, headers_applicator);
  ASSERT_NE(nullptr, exporter1);

  auto exporter2 = cache->getOrCreateLogger(config, http_service, headers_applicator);
  EXPECT_EQ(exporter1.get(), exporter2.get());
}

TEST(OtlpHttpLogsExporterCacheTest, CreateApplicatorFailure) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/otlp/v1/logs"
    cluster: "my_o11y_backend"
    timeout: 0.250s
  request_headers_to_add:
    - header:
        key: "x-bad-formatter"
        value: "%UNCLOSED_FORMATTER"
  )EOF";

  envoy::config::core::v3::HttpService http_service;
  TestUtility::loadFromYaml(yaml_string, http_service);

  NiceMock<Server::Configuration::MockServerFactoryContext> server_context;

  auto cache = std::make_shared<OtlpHttpLogsExporterCache>(server_context);

  EXPECT_THROW(cache->getOrCreateApplicator(http_service, server_context), EnvoyException);
}

} // namespace
} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

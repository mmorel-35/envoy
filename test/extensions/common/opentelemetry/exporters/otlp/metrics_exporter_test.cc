#include "source/common/tracing/null_span_impl.h"
#include "source/extensions/common/opentelemetry/exporters/otlp/grpc_metrics_exporter.h"
#include "source/extensions/common/opentelemetry/exporters/otlp/http_metrics_exporter.h"

#include "test/mocks/grpc/mocks.h"
#include "test/mocks/server/server_factory_context.h"
#include "test/mocks/upstream/cluster_manager.h"
#include "test/test_common/utility.h"

#include "absl/strings/match.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {
namespace {

using testing::_;
using testing::Invoke;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;

// --------------------------------------------------------------------------
// gRPC metrics exporter tests
// --------------------------------------------------------------------------

class OtlpGrpcMetricsExporterImplTest : public testing::Test {
public:
  OtlpGrpcMetricsExporterImplTest() {
    exporter_ = std::make_unique<OtlpGrpcMetricsExporterImpl>(
        Grpc::RawAsyncClientSharedPtr{async_client_});
  }

  Grpc::MockAsyncClient* async_client_{new NiceMock<Grpc::MockAsyncClient>};
  std::unique_ptr<OtlpGrpcMetricsExporterImpl> exporter_;
};

TEST_F(OtlpGrpcMetricsExporterImplTest, SendExportRequest) {
  EXPECT_CALL(*async_client_, sendRaw(_, _, _, _, _, _));
  exporter_->send(std::make_unique<MetricsExportRequest>());
}

TEST_F(OtlpGrpcMetricsExporterImplTest, OnSuccessNoPartialSuccess) {
  auto response = std::make_unique<MetricsExportResponse>();
  // No partial_success field set - should complete without error.
  exporter_->onSuccess(std::move(response), Tracing::NullSpan::instance());
}

TEST_F(OtlpGrpcMetricsExporterImplTest, OnSuccessWithPartialSuccess) {
  auto response = std::make_unique<MetricsExportResponse>();
  response->mutable_partial_success()->set_rejected_data_points(3);
  response->mutable_partial_success()->set_error_message("some data points rejected");
  exporter_->onSuccess(std::move(response), Tracing::NullSpan::instance());
}

TEST_F(OtlpGrpcMetricsExporterImplTest, OnFailure) {
  exporter_->onFailure(Grpc::Status::WellKnownGrpcStatus::Internal, "internal error",
                       Tracing::NullSpan::instance());
}

TEST_F(OtlpGrpcMetricsExporterImplTest, OnCreateInitialMetadata) {
  Http::TestRequestHeaderMapImpl headers;
  exporter_->onCreateInitialMetadata(headers);
  // onCreateInitialMetadata is a no-op; verify no crash.
}

// --------------------------------------------------------------------------
// HTTP metrics exporter tests
// --------------------------------------------------------------------------

class OtlpHttpMetricsExporterTest : public testing::Test {
public:
  void setup(envoy::config::core::v3::HttpService http_service) {
    cluster_manager_.thread_local_cluster_.cluster_.info_->name_ = "my_o11y_backend";
    cluster_manager_.initializeThreadLocalClusters({"my_o11y_backend"});
    ON_CALL(cluster_manager_.thread_local_cluster_, httpAsyncClient())
        .WillByDefault(ReturnRef(cluster_manager_.thread_local_cluster_.async_client_));
    cluster_manager_.initializeClusters({"my_o11y_backend"}, {});

    http_exporter_ = std::make_unique<OtlpHttpMetricsExporter>(cluster_manager_, http_service,
                                                                server_context_);
  }

  MetricsExportRequestPtr createTestMetricsRequest() {
    auto request = std::make_unique<MetricsExportRequest>();
    auto* resource_metrics = request->add_resource_metrics();
    auto* scope_metrics = resource_metrics->add_scope_metrics();
    auto* metric = scope_metrics->add_metrics();
    metric->set_name("test_metric");
    auto* gauge = metric->mutable_gauge();
    auto* data_point = gauge->add_data_points();
    data_point->set_as_int(42);
    return request;
  }

protected:
  NiceMock<Server::Configuration::MockServerFactoryContext> server_context_;
  NiceMock<Upstream::MockClusterManager> cluster_manager_;
  std::unique_ptr<OtlpHttpMetricsExporter> http_exporter_;
};

TEST_F(OtlpHttpMetricsExporterTest, ExportMetricsWithCustomHeaders) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/v1/metrics"
    cluster: "my_o11y_backend"
    timeout: 0.250s
  request_headers_to_add:
  - header:
      key: "Authorization"
      value: "auth-token"
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
            EXPECT_EQ("/v1/metrics", message->headers().getPathValue());
            EXPECT_EQ("some-o11y.com", message->headers().getHostValue());
            EXPECT_TRUE(absl::StartsWith(message->headers().getUserAgentValue(),
                                         "OTel-OTLP-Exporter-Envoy/"));
            EXPECT_EQ("auth-token", message->headers()
                                        .get(Http::LowerCaseString("authorization"))[0]
                                        ->value()
                                        .getStringView());

            return &request;
          }));

  http_exporter_->send(createTestMetricsRequest());

  Http::ResponseMessagePtr msg(new Http::ResponseMessageImpl(
      Http::ResponseHeaderMapPtr{new Http::TestResponseHeaderMapImpl{{":status", "200"}}}));

  Tracing::NullSpan null_span;
  callback->onBeforeFinalizeUpstreamSpan(null_span, nullptr);
  callback->onSuccess(request, std::move(msg));
}

TEST_F(OtlpHttpMetricsExporterTest, ExportWithoutCluster) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/v1/metrics"
    cluster: "my_o11y_backend"
    timeout: 10s
  )EOF";

  envoy::config::core::v3::HttpService http_service;
  TestUtility::loadFromYaml(yaml_string, http_service);
  setup(http_service);

  ON_CALL(cluster_manager_, getThreadLocalCluster(absl::string_view("my_o11y_backend")))
      .WillByDefault(Return(nullptr));

  http_exporter_->send(createTestMetricsRequest());
}

TEST_F(OtlpHttpMetricsExporterTest, ExportMetricsNonSuccessStatusCode) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/v1/metrics"
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

  http_exporter_->send(createTestMetricsRequest());

  Http::ResponseMessagePtr msg(new Http::ResponseMessageImpl(
      Http::ResponseHeaderMapPtr{new Http::TestResponseHeaderMapImpl{{":status", "503"}}}));
  callback->onSuccess(request, std::move(msg));
}

TEST_F(OtlpHttpMetricsExporterTest, ExportMetricsHttpFailure) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/v1/metrics"
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

  http_exporter_->send(createTestMetricsRequest());
  callback->onFailure(request, Http::AsyncClient::FailureReason::Reset);
}

TEST_F(OtlpHttpMetricsExporterTest, SendReturnsNullptr) {
  std::string yaml_string = R"EOF(
  http_uri:
    uri: "https://some-o11y.com/v1/metrics"
    cluster: "my_o11y_backend"
    timeout: 0.250s
  )EOF";

  envoy::config::core::v3::HttpService http_service;
  TestUtility::loadFromYaml(yaml_string, http_service);
  setup(http_service);

  EXPECT_CALL(cluster_manager_.thread_local_cluster_.async_client_, send_(_, _, _))
      .WillOnce(Return(nullptr));

  http_exporter_->send(createTestMetricsRequest());
}

} // namespace
} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

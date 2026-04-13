#pragma once

#include "envoy/singleton/instance.h"

#include "source/common/grpc/typed_async_client.h"
#include "source/extensions/common/opentelemetry/exporters/otlp/metrics_exporter.h"

#include "opentelemetry/proto/collector/metrics/v1/metrics_service.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

/**
 * gRPC implementation of OtlpMetricsExporter.
 * Handles the gRPC async callbacks in addition to the export interface.
 */
class OtlpGrpcMetricsExporter : public OtlpMetricsExporter,
                                 public Grpc::AsyncRequestCallbacks<MetricsExportResponse> {
public:
  ~OtlpGrpcMetricsExporter() override = default;

  // Grpc::AsyncRequestCallbacks
  void onCreateInitialMetadata(Http::RequestHeaderMap&) override {}
};

using OtlpGrpcMetricsExporterSharedPtr = std::shared_ptr<OtlpGrpcMetricsExporter>;

/**
 * Production implementation of OtlpGrpcMetricsExporter.
 */
class OtlpGrpcMetricsExporterImpl : public Singleton::Instance,
                                     public OtlpGrpcMetricsExporter,
                                     public Logger::Loggable<Logger::Id::stats> {
public:
  OtlpGrpcMetricsExporterImpl(Grpc::RawAsyncClientSharedPtr raw_async_client);

  // OtlpMetricsExporter
  void send(MetricsExportRequestPtr&& metrics) override;

  // Grpc::AsyncRequestCallbacks
  void onSuccess(Grpc::ResponsePtr<MetricsExportResponse>&&, Tracing::Span&) override;
  void onFailure(Grpc::Status::GrpcStatus, const std::string&, Tracing::Span&) override;

private:
  Grpc::AsyncClient<MetricsExportRequest, MetricsExportResponse> client_;
  const Protobuf::MethodDescriptor& service_method_;
};

using OtlpGrpcMetricsExporterImplPtr = std::unique_ptr<OtlpGrpcMetricsExporterImpl>;

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

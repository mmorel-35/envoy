#pragma once

#include "source/common/grpc/typed_async_client.h"
#include "source/extensions/common/opentelemetry/exporters/otlp/trace_exporter.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {

/**
 * gRPC implementation of the OTLP trace exporter.
 *
 * Mirrors opentelemetry::exporter::otlp::OtlpGrpcExporter, adapted for Envoy's gRPC async client.
 * @see https://github.com/open-telemetry/opentelemetry-cpp/blob/main/exporters/otlp/include/opentelemetry/exporters/otlp/otlp_grpc_exporter.h
 */
class OtlpGrpcTraceExporter : public OtlpTraceExporter,
                               public Grpc::AsyncRequestCallbacks<TraceExportResponse> {
public:
  OtlpGrpcTraceExporter(const Grpc::RawAsyncClientSharedPtr& client);
  ~OtlpGrpcTraceExporter() override = default;

  void onCreateInitialMetadata(Http::RequestHeaderMap& metadata) override;

  void onSuccess(Grpc::ResponsePtr<TraceExportResponse>&& response, Tracing::Span&) override;

  void onFailure(Grpc::Status::GrpcStatus status, const std::string& message,
                 Tracing::Span&) override;

  bool log(const TraceExportRequest& request) override;

  Grpc::AsyncClient<TraceExportRequest, TraceExportResponse> client_;
  const Protobuf::MethodDescriptor& service_method_;
};

} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

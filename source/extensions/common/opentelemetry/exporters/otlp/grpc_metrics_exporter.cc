#include "source/extensions/common/opentelemetry/exporters/otlp/grpc_metrics_exporter.h"

#include "source/common/tracing/null_span_impl.h"
#include "source/extensions/common/opentelemetry/sdk/metrics/constants.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

OtlpGrpcMetricsExporterImpl::OtlpGrpcMetricsExporterImpl(
    Grpc::RawAsyncClientSharedPtr raw_async_client)
    : client_(raw_async_client),
      service_method_(*Protobuf::DescriptorPool::generated_pool()->FindMethodByName(
          std::string(Envoy::Extensions::OpenTelemetry::Sdk::Metrics::Constants::
                          kMetricsServiceExportMethod))) {}

void OtlpGrpcMetricsExporterImpl::send(MetricsExportRequestPtr&& export_request) {
  ENVOY_LOG(debug, "sending a OTLP metric request: {}", export_request->DebugString());
  client_->send(service_method_, *export_request, *this, Tracing::NullSpan::instance(),
                Http::AsyncClient::RequestOptions());
}

void OtlpGrpcMetricsExporterImpl::onSuccess(
    Grpc::ResponsePtr<MetricsExportResponse>&& export_response, Tracing::Span&) {
  if (export_response->has_partial_success()) {
    ENVOY_LOG(debug,
              "export response with partial success; {} rejected, collector "
              "message: {}",
              export_response->partial_success().rejected_data_points(),
              export_response->partial_success().error_message());
  }
}

void OtlpGrpcMetricsExporterImpl::onFailure(Grpc::Status::GrpcStatus response_status,
                                             const std::string& response_message, Tracing::Span&) {
  ENVOY_LOG(debug, "export failure; status: {}, message: {}", response_status, response_message);
}

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

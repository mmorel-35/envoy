#pragma once

#include "envoy/config/core/v3/http_service.pb.h"
#include "envoy/server/factory_context.h"
#include "envoy/upstream/cluster_manager.h"

#include "source/common/common/logger.h"
#include "source/common/http/async_client_impl.h"
#include "source/common/http/async_client_utility.h"
#include "source/common/http/http_service_headers.h"
#include "source/common/http/message_impl.h"
#include "source/common/http/utility.h"
#include "source/extensions/common/opentelemetry/exporters/otlp/trace_exporter.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {

/**
 * HTTP implementation of the OTLP trace exporter.
 *
 * Mirrors opentelemetry::exporter::otlp::OtlpHttpExporter, adapted for Envoy's HTTP async client.
 * @see https://github.com/open-telemetry/opentelemetry-cpp/blob/main/exporters/otlp/include/opentelemetry/exporters/otlp/otlp_http_exporter.h
 */
class OtlpHttpTraceExporter : public OtlpTraceExporter, public Http::AsyncClient::Callbacks {
public:
  OtlpHttpTraceExporter(
      Upstream::ClusterManager& cluster_manager,
      const envoy::config::core::v3::HttpService& http_service,
      std::shared_ptr<const Http::HttpServiceHeadersApplicator> headers_applicator);

  bool log(const TraceExportRequest& request) override;

  // Http::AsyncClient::Callbacks.
  void onSuccess(const Http::AsyncClient::Request&, Http::ResponseMessagePtr&&) override;
  void onFailure(const Http::AsyncClient::Request&, Http::AsyncClient::FailureReason) override;
  void onBeforeFinalizeUpstreamSpan(Tracing::Span&, const Http::ResponseHeaderMap*) override {}

private:
  Upstream::ClusterManager& cluster_manager_;
  envoy::config::core::v3::HttpService http_service_;
  // Track active HTTP requests to be able to cancel them on destruction.
  Http::AsyncClientRequestTracker active_requests_;
  std::shared_ptr<const Http::HttpServiceHeadersApplicator> headers_applicator_;
};

} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

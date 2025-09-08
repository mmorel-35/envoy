#pragma once

#include "source/common/common/logger.h"
#include "source/extensions/opentelemetry/sdk/common/types.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

// Type aliases for backward compatibility
using ExportTraceServiceRequest = ::Envoy::Extensions::OpenTelemetry::Sdk::Common::TraceExportRequest;
using ExportTraceServiceResponse = ::Envoy::Extensions::OpenTelemetry::Sdk::Common::TraceExportResponse;

/**
 * @brief Base class for all OpenTelemetry Protocol (OTLP) exporters.
 * @see
 * https://github.com/open-telemetry/opentelemetry-proto/blob/v1.0.0/docs/specification.md#otlphttp
 */
class OpenTelemetryTraceExporter : public Logger::Loggable<Logger::Id::tracing> {
public:
  virtual ~OpenTelemetryTraceExporter() = default;

  /**
   * @brief Exports the trace request to the configured OTLP service.
   *
   * @param request The protobuf-encoded OTLP trace request.
   * @return true When the request was sent.
   * @return false When sending the request failed.
   */
  virtual bool log(const ExportTraceServiceRequest& request) = 0;

  /**
   * @brief Logs as debug the number of exported spans.
   *
   * @param request The protobuf-encoded OTLP trace request.
   */
  void logExportedSpans(const ExportTraceServiceRequest& request) {
    if (request.resource_spans(0).has_resource()) {
      if (request.resource_spans(0).scope_spans(0).has_scope()) {
        ENVOY_LOG(debug, "Number of exported spans: {}",
                  request.resource_spans(0).scope_spans(0).spans_size());
      }
    }
  }
};

using OpenTelemetryTraceExporterPtr = std::unique_ptr<OpenTelemetryTraceExporter>;

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

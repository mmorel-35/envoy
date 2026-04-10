#pragma once

#include <memory>

#include "source/common/common/logger.h"

#include "opentelemetry/proto/collector/trace/v1/trace_service.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

/**
 * @brief Base interface for all Envoy OTLP trace exporters.
 *
 * Mirrors opentelemetry::sdk::trace::SpanExporter from opentelemetry-cpp, adapted to Envoy's
 * async I/O model and proto-based OTLP wire format.
 * @see https://github.com/open-telemetry/opentelemetry-cpp/blob/main/sdk/include/opentelemetry/sdk/trace/exporter.h
 */
class OtlpTraceExporter : public Logger::Loggable<Logger::Id::tracing> {
public:
  virtual ~OtlpTraceExporter() = default;

  /**
   * @brief Exports the trace request to the configured OTLP service.
   *
   * @param request The protobuf-encoded OTLP trace request.
   * @return true When the request was sent.
   * @return false When sending the request failed.
   */
  virtual bool
  log(const opentelemetry::proto::collector::trace::v1::ExportTraceServiceRequest& request) = 0;

  /**
   * @brief Logs as debug the number of exported spans.
   *
   * @param request The protobuf-encoded OTLP trace request.
   */
  void logExportedSpans(
      const opentelemetry::proto::collector::trace::v1::ExportTraceServiceRequest& request) {
    if (request.resource_spans_size() == 0) {
      return;
    }
    const auto& resource_spans = request.resource_spans(0);
    if (resource_spans.scope_spans_size() == 0) {
      return;
    }
    const auto& scope_spans = resource_spans.scope_spans(0);
    if (resource_spans.has_resource()) {
      if (scope_spans.has_scope()) {
        ENVOY_LOG(debug, "Number of exported spans: {}", scope_spans.spans_size());
      }
    }
  }
};

using OtlpTraceExporterPtr = std::unique_ptr<OtlpTraceExporter>;

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy


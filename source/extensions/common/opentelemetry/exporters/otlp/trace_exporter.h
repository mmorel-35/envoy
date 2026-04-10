#pragma once

#include "source/common/common/logger.h"
#include "source/extensions/common/opentelemetry/sdk/trace/types.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {

/**
 * @brief Base interface for all Envoy OTLP trace exporters.
 *
 * Mirrors opentelemetry::sdk::trace::SpanExporter, adapted for Envoy's async I/O model.
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
  virtual bool log(const TraceExportRequest& request) = 0;

  /**
   * @brief Logs as debug the number of exported spans.
   *
   * @param request The protobuf-encoded OTLP trace request.
   */
  void logExportedSpans(const TraceExportRequest& request) {
    if (request.resource_spans(0).has_resource()) {
      if (request.resource_spans(0).scope_spans(0).has_scope()) {
        ENVOY_LOG(debug, "Number of exported spans: {}",
                  request.resource_spans(0).scope_spans(0).spans_size());
      }
    }
  }
};

using OtlpTraceExporterPtr = std::unique_ptr<OtlpTraceExporter>;

} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

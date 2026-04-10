#pragma once

#include <memory>

#include "opentelemetry/proto/collector/metrics/v1/metrics_service.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

using MetricsExportRequest =
    opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using MetricsExportRequestPtr = std::unique_ptr<MetricsExportRequest>;
using MetricsExportResponse =
    opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceResponse;

/**
 * Abstract base class for OTLP metrics exporters.
 */
class OtlpMetricsExporter {
public:
  virtual ~OtlpMetricsExporter() = default;

  /**
   * Send metrics to the configured OTLP service.
   * @param metrics the OTLP metrics export request.
   */
  virtual void send(MetricsExportRequestPtr&& metrics) PURE;
};

using OtlpMetricsExporterSharedPtr = std::shared_ptr<OtlpMetricsExporter>;

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

#pragma once

#include <memory>

#include "opentelemetry/proto/collector/logs/v1/logs_service.pb.h"
#include "opentelemetry/proto/logs/v1/logs.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

using LogsExportRequest =
    opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest;
using LogsExportRequestPtr = std::unique_ptr<LogsExportRequest>;
using LogsExportResponse =
    opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse;

/**
 * Abstract base class for OTLP logs exporters.
 */
class OtlpLogsExporter {
public:
  virtual ~OtlpLogsExporter() = default;

  /**
   * Log a log record entry to the configured OTLP service.
   * @param entry the OTLP log record.
   */
  virtual void log(opentelemetry::proto::logs::v1::LogRecord&& entry) PURE;
};

using OtlpLogsExporterSharedPtr = std::shared_ptr<OtlpLogsExporter>;

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

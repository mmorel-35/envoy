#pragma once

#include <map>
#include <memory>
#include <string>

#include "opentelemetry/proto/collector/logs/v1/logs_service.pb.h"
#include "opentelemetry/proto/common/v1/common.pb.h"
#include "opentelemetry/proto/logs/v1/logs.pb.h"

namespace Envoy {
namespace Common {
namespace OpenTelemetry {
namespace Logs {

/**
 * @brief Common key-value pair type used in OpenTelemetry logs
 */
using KeyValue = ::opentelemetry::proto::common::v1::KeyValue;

// OTLP Log Export Request Types
using LogsExportRequest = ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest;
using LogsExportResponse = ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse;

// Smart pointer aliases for log export requests
using LogsExportRequestPtr = std::unique_ptr<LogsExportRequest>;
using LogsExportRequestSharedPtr = std::shared_ptr<LogsExportRequest>;

} // namespace Logs
} // namespace OpenTelemetry
} // namespace Common
} // namespace Envoy
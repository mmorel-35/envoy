#pragma once

#include <map>
#include <memory>
#include <string>

#include "opentelemetry/common/attribute_value.h"
#include "opentelemetry/proto/collector/logs/v1/logs_service.pb.h"
#include "opentelemetry/proto/collector/metrics/v1/metrics_service.pb.h"
#include "opentelemetry/proto/collector/trace/v1/trace_service.pb.h"
#include "opentelemetry/proto/common/v1/common.pb.h"
#include "opentelemetry/proto/metrics/v1/metrics.pb.h"
#include "opentelemetry/proto/trace/v1/trace.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Sdk {
namespace Common {

// Common OpenTelemetry type aliases used across all telemetry signals

/**
 * @brief The type of the span.
 * see
 * https://github.com/open-telemetry/opentelemetry-specification/blob/main/specification/trace/api.md#spankind
 */
using OTelSpanKind = ::opentelemetry::proto::trace::v1::Span::SpanKind;

/**
 * @brief Open-telemetry Attribute
 * see
 * https://github.com/open-telemetry/opentelemetry-cpp/blob/main/api/include/opentelemetry/common/attribute_value.h
 */
using OTelAttribute = ::opentelemetry::common::AttributeValue;

/**
 * @brief Container holding Open-telemetry Attributes
 */
using OTelAttributes = std::map<std::string, OTelAttribute>;

/**
 * @brief Common key-value pair type used in OpenTelemetry
 */
using KeyValue = ::opentelemetry::proto::common::v1::KeyValue;

/**
 * @brief Aggregation temporality for metrics
 */
using AggregationTemporality = ::opentelemetry::proto::metrics::v1::AggregationTemporality;

// OTLP Export Request Types
using TraceExportRequest = ::opentelemetry::proto::collector::trace::v1::ExportTraceServiceRequest;
using TraceExportResponse =
    ::opentelemetry::proto::collector::trace::v1::ExportTraceServiceResponse;

using MetricsExportRequest =
    ::opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using MetricsExportResponse =
    ::opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceResponse;

using LogsExportRequest = ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest;
using LogsExportResponse = ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse;

// Smart pointer aliases for export requests
using TraceExportRequestPtr = std::unique_ptr<TraceExportRequest>;
using TraceExportRequestSharedPtr = std::shared_ptr<TraceExportRequest>;

using MetricsExportRequestPtr = std::unique_ptr<MetricsExportRequest>;
using MetricsExportRequestSharedPtr = std::shared_ptr<MetricsExportRequest>;

using LogsExportRequestPtr = std::unique_ptr<LogsExportRequest>;
using LogsExportRequestSharedPtr = std::shared_ptr<LogsExportRequest>;

} // namespace Common
} // namespace Sdk
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy
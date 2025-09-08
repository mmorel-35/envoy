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

/**
 * Common OpenTelemetry type aliases used across all telemetry signals.
 * 
 * Organization: Types are grouped by OpenTelemetry signal (trace, metrics, logs, common)
 * Origin: All types are derived from official OpenTelemetry C++ SDK and protocol definitions
 * Reference: https://github.com/open-telemetry/opentelemetry-cpp
 */

// === COMMON TYPES ===
// Types used across multiple telemetry signals (from OpenTelemetry C++ SDK)

/**
 * @brief Open-telemetry Attribute (from OpenTelemetry C++ SDK)
 * @see https://github.com/open-telemetry/opentelemetry-cpp/blob/main/api/include/opentelemetry/common/attribute_value.h
 */
using OTelAttribute = ::opentelemetry::common::AttributeValue;

/**
 * @brief Container holding Open-telemetry Attributes (Envoy extension)
 */
using OTelAttributes = std::map<std::string, OTelAttribute>;

/**
 * @brief Common key-value pair type used in OpenTelemetry (from OTLP spec)
 */
using KeyValue = ::opentelemetry::proto::common::v1::KeyValue;

// === TRACE SIGNAL ===
// Types specific to trace telemetry signal (from OpenTelemetry specification)

/**
 * @brief The type of the span (from OpenTelemetry specification)
 * @see https://github.com/open-telemetry/opentelemetry-specification/blob/main/specification/trace/api.md#spankind
 */
using OTelSpanKind = ::opentelemetry::proto::trace::v1::Span::SpanKind;

// OTLP export request/response types for traces (from OTLP protocol)
using TraceExportRequest = ::opentelemetry::proto::collector::trace::v1::ExportTraceServiceRequest;
using TraceExportResponse = ::opentelemetry::proto::collector::trace::v1::ExportTraceServiceResponse;

// Smart pointer aliases for trace export requests (Envoy convenience types)
using TraceExportRequestPtr = std::unique_ptr<TraceExportRequest>;
using TraceExportRequestSharedPtr = std::shared_ptr<TraceExportRequest>;

// === METRICS SIGNAL ===
// Types specific to metrics telemetry signal (from OpenTelemetry specification)

/**
 * @brief Aggregation temporality for metrics (from OpenTelemetry specification)
 */
using AggregationTemporality = ::opentelemetry::proto::metrics::v1::AggregationTemporality;

// OTLP export request/response types for metrics (from OTLP protocol)
using MetricsExportRequest = ::opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using MetricsExportResponse = ::opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceResponse;

// Smart pointer aliases for metrics export requests (Envoy convenience types)
using MetricsExportRequestPtr = std::unique_ptr<MetricsExportRequest>;
using MetricsExportRequestSharedPtr = std::shared_ptr<MetricsExportRequest>;

// === LOGS SIGNAL ===
// Types specific to logs telemetry signal (from OTLP protocol)

using LogsExportRequest = ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest;
using LogsExportResponse = ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse;

// Smart pointer aliases for logs export requests (Envoy convenience types)
using LogsExportRequestPtr = std::unique_ptr<LogsExportRequest>;
using LogsExportRequestSharedPtr = std::shared_ptr<LogsExportRequest>;

} // namespace Common
} // namespace Sdk
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

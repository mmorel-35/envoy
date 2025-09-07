#pragma once

#include <memory>
#include <string>

#include "opentelemetry/proto/collector/metrics/v1/metrics_service.pb.h"
#include "opentelemetry/proto/common/v1/common.pb.h"
#include "opentelemetry/proto/metrics/v1/metrics.pb.h"

namespace Envoy {
namespace Common {
namespace OpenTelemetry {
namespace Metrics {

/**
 * @brief Common key-value pair type used in OpenTelemetry metrics
 */
using KeyValue = ::opentelemetry::proto::common::v1::KeyValue;

/**
 * @brief Aggregation temporality for metrics
 */
using AggregationTemporality = ::opentelemetry::proto::metrics::v1::AggregationTemporality;

// OTLP Metrics Export Request Types
using MetricsExportRequest =
    ::opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using MetricsExportResponse =
    ::opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceResponse;

// Smart pointer aliases for metrics export requests
using MetricsExportRequestPtr = std::unique_ptr<MetricsExportRequest>;
using MetricsExportRequestSharedPtr = std::shared_ptr<MetricsExportRequest>;

} // namespace Metrics
} // namespace OpenTelemetry
} // namespace Common
} // namespace Envoy
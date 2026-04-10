#pragma once

#include <memory>

#include "opentelemetry/proto/collector/metrics/v1/metrics_service.pb.h"
#include "opentelemetry/proto/metrics/v1/metrics.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {

/**
 * @brief Aggregation temporality for metrics (from OpenTelemetry specification)
 */
using AggregationTemporality = ::opentelemetry::proto::metrics::v1::AggregationTemporality;

// OTLP export request/response types for metrics (from OTLP protocol)
using MetricsExportRequest =
    ::opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using MetricsExportResponse =
    ::opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceResponse;

// Smart pointer aliases for metrics export requests (Envoy convenience types)
using MetricsExportRequestPtr = std::unique_ptr<MetricsExportRequest>;
using MetricsExportRequestSharedPtr = std::shared_ptr<MetricsExportRequest>;

} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

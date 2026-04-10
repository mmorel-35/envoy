#pragma once

#include <memory>

#include "opentelemetry/proto/collector/trace/v1/trace_service.pb.h"
#include "opentelemetry/proto/trace/v1/trace.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {

/**
 * @brief The type of the span.
 * @see
 * https://github.com/open-telemetry/opentelemetry-specification/blob/main/specification/trace/api.md#spankind
 */
using SpanKind = ::opentelemetry::proto::trace::v1::Span::SpanKind;

// OTLP export request/response types for traces (from OTLP protocol)
using TraceExportRequest =
    ::opentelemetry::proto::collector::trace::v1::ExportTraceServiceRequest;
using TraceExportResponse =
    ::opentelemetry::proto::collector::trace::v1::ExportTraceServiceResponse;

// Smart pointer aliases for trace export requests (Envoy convenience types)
using TraceExportRequestPtr = std::unique_ptr<TraceExportRequest>;
using TraceExportRequestSharedPtr = std::shared_ptr<TraceExportRequest>;

} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy

#pragma once

#include <map>
#include <memory>
#include <string>

#include "opentelemetry/common/attribute_value.h"
#include "opentelemetry/proto/collector/trace/v1/trace_service.pb.h"
#include "opentelemetry/proto/common/v1/common.pb.h"
#include "opentelemetry/proto/trace/v1/trace.pb.h"

namespace Envoy {
namespace Common {
namespace OpenTelemetry {
namespace Traces {

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
 * @brief Common key-value pair type used in OpenTelemetry traces
 */
using KeyValue = ::opentelemetry::proto::common::v1::KeyValue;

// OTLP Trace Export Request Types
using TraceExportRequest = ::opentelemetry::proto::collector::trace::v1::ExportTraceServiceRequest;
using TraceExportResponse =
    ::opentelemetry::proto::collector::trace::v1::ExportTraceServiceResponse;

// Smart pointer aliases for trace export requests
using TraceExportRequestPtr = std::unique_ptr<TraceExportRequest>;
using TraceExportRequestSharedPtr = std::shared_ptr<TraceExportRequest>;

} // namespace Traces
} // namespace OpenTelemetry
} // namespace Common
} // namespace Envoy
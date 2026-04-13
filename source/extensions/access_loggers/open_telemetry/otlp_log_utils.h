#pragma once

// Compatibility header - all utilities are now in shared OTLP logs_utils.h.
#include "source/extensions/common/opentelemetry/exporters/otlp/logs_utils.h"

namespace Envoy {
namespace Extensions {
namespace AccessLoggers {
namespace OpenTelemetry {

// Re-export all symbols from shared OTLP into this namespace for backward compatibility.
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::DefaultBufferFlushInterval;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::DefaultMaxBufferSizeBytes;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::BodyKey;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::TraceIdHexLength;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::ShortTraceIdHexLength;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OtlpAccessLogStatsPrefix;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OtlpAccessLogStats;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::packBody;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::unpackBody;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::populateTraceContext;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::getLogName;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::getGrpcService;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::getBufferFlushInterval;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::getBufferSizeBytes;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::getFilterStateObjectsToLog;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::getCustomTags;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::addFilterStateToAttributes;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::addCustomTagsToAttributes;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::initOtlpMessageRoot;
using ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::getStringKeyValue;

} // namespace OpenTelemetry
} // namespace AccessLoggers
} // namespace Extensions
} // namespace Envoy

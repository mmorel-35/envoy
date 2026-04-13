#pragma once

// Compatibility header - the OTLP gRPC logs exporter now lives in shared OTLP.
#include "source/extensions/common/opentelemetry/exporters/otlp/grpc_logs_exporter.h"

namespace Envoy {
namespace Extensions {
namespace AccessLoggers {
namespace OpenTelemetry {

// Backward-compatible aliases pointing to the shared OTLP gRPC logs exporter.
using GrpcAccessLoggerImpl =
    ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OtlpGrpcLogsExporter;
using GrpcAccessLoggerCacheImpl =
    ::Envoy::Extensions::OpenTelemetry::Exporters::Otlp::OtlpGrpcLogsExporterCache;

using GrpcAccessLogger = GrpcAccessLoggerImpl::Interface;
using GrpcAccessLoggerSharedPtr = GrpcAccessLogger::SharedPtr;

using GrpcAccessLoggerCache = GrpcAccessLoggerCacheImpl::Interface;
using GrpcAccessLoggerCacheSharedPtr = GrpcAccessLoggerCache::SharedPtr;

} // namespace OpenTelemetry
} // namespace AccessLoggers
} // namespace Extensions
} // namespace Envoy

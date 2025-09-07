# OpenTelemetry Code Mutualization Plan for Envoy

## Executive Summary

This document provides a comprehensive analysis and actionable plan for consolidating OpenTelemetry-related code in Envoy. Currently, OpenTelemetry functionality is fragmented across tracers, stat sinks, and access loggers with significant code duplication. This plan proposes creating a centralized `source/common/opentelemetry/` directory to share common constants, utilities, and protocol logic.

**Related Issue**: [envoyproxy/envoy#41010](https://github.com/envoyproxy/envoy/issues/41010)
**Referenced PR**: [envoyproxy/envoy#40989](https://github.com/envoyproxy/envoy/pull/40989) (propagator refactoring)

---

## Current State Analysis

### 1. OpenTelemetry Code Locations

Based on comprehensive code survey, OpenTelemetry functionality exists in the following locations:

#### A. Tracing Extensions (`source/extensions/tracers/opentelemetry/`)
- **57+ files** including main tracer implementation
- **Key components**:
  - `otlp_utils.h/cc` - OTLP utilities and type definitions
  - Span context handling and W3C traceparent/tracestate support
  - Resource detectors (static, environment, dynatrace)
  - Samplers (always_on, cel, dynatrace, parent_based, trace_id_ratio_based)
- **Proto usage**: `opentelemetry.proto.collector.trace.v1.TraceService.Export`
- **Type aliases**: `OTelSpanKind`, `OTelAttribute`, `OtelAttributes`

#### B. Stat Sinks (`source/extensions/stat_sinks/open_telemetry/`)
- **6 files** for OTLP metrics export
- **Key components**:
  - OTLP metrics flusher and GRPC exporter
  - Proto descriptors validation
- **Proto usage**: `opentelemetry.proto.collector.metrics.v1.MetricsService.Export`
- **Type aliases**: `AggregationTemporality`, `MetricsExportRequest`, `KeyValue`

#### C. Access Loggers (`source/extensions/access_loggers/open_telemetry/`)
- **10 files** for OTLP logging
- **Key components**:
  - OTLP logging implementation and GRPC exporter
  - Proto descriptors validation
  - Helper functions like `getStringKeyValue`
- **Proto usage**: `opentelemetry.proto.collector.logs.v1.LogsService.Export`

#### D. Common Tracing (`source/common/tracing/`)
- **TraceContextHandler** for managing trace headers
- **Common trace tag values** but no OpenTelemetry-specific code
- **W3C traceparent/tracestate** support infrastructure

### 2. Code Duplication Analysis

#### Critical Duplications Found:

1. **OTLP Service Method Strings** (3x duplication):
   ```cpp
   // Tracers
   "opentelemetry.proto.collector.trace.v1.TraceService.Export"
   // Stats
   "opentelemetry.proto.collector.metrics.v1.MetricsService.Export"  
   // Logs
   "opentelemetry.proto.collector.logs.v1.LogsService.Export"
   ```

2. **Proto Descriptors Validation** (3x duplication):
   - Each extension has identical `validateProtoDescriptors()` function
   - Same pattern but different service method strings

3. **Type Aliases Fragmentation**:
   - Tracers: `OTelSpanKind`, `OTelAttribute`, `OtelAttributes`
   - Stats: `AggregationTemporality`, `MetricsExportRequest`, `KeyValue`
   - No shared type definitions across telemetry signals

4. **OTLP Utilities Isolation**:
   - `OtlpUtils::getOtlpUserAgentHeader()` only in tracers
   - `OtlpUtils::populateAnyValue()` only in tracers
   - Could benefit stats and logs exporters

5. **Environment Variable Support**:
   - Only `OTEL_RESOURCE_ATTRIBUTES` in resource detectors
   - Missing `OTEL_PROPAGATORS`, `OTEL_SERVICE_NAME`, etc.

---

## Proposed Mutualization Plan

### 1. Create Centralized Directory Structure

```
source/common/opentelemetry/
├── BUILD
├── otlp_constants.h              # OTLP service method strings and endpoints
├── otlp_types.h                  # Common OpenTelemetry type aliases  
├── otlp_utils.h                  # Shared OTLP utilities
├── otlp_utils.cc                 # Implementation
├── proto_validation.h            # Centralized proto descriptors validation
├── proto_validation.cc           # Implementation
├── environment.h                 # OTEL_* environment variable support
├── environment.cc                # Implementation
└── resource/                     # Shared resource handling
    ├── BUILD
    ├── common_attributes.h        # Common resource attributes
    └── common_attributes.cc       # Implementation
```

### 2. Detailed File Contents

#### A. `otlp_constants.h` - Protocol Constants
```cpp
namespace Envoy {
namespace Common {
namespace OpenTelemetry {

class OtlpServiceMethods {
public:
  static constexpr absl::string_view TRACE_EXPORT = 
    "opentelemetry.proto.collector.trace.v1.TraceService.Export";
  static constexpr absl::string_view METRICS_EXPORT = 
    "opentelemetry.proto.collector.metrics.v1.MetricsService.Export";
  static constexpr absl::string_view LOGS_EXPORT = 
    "opentelemetry.proto.collector.logs.v1.LogsService.Export";
};

class OtlpEndpoints {
public:
  static constexpr absl::string_view DEFAULT_TRACE_ENDPOINT = "/v1/traces";
  static constexpr absl::string_view DEFAULT_METRICS_ENDPOINT = "/v1/metrics"; 
  static constexpr absl::string_view DEFAULT_LOGS_ENDPOINT = "/v1/logs";
};

} // namespace OpenTelemetry
} // namespace Common  
} // namespace Envoy
```

#### B. `otlp_types.h` - Shared Type Aliases
```cpp
namespace Envoy {
namespace Common {
namespace OpenTelemetry {

// Common OpenTelemetry types across all signals
using OTelSpanKind = ::opentelemetry::proto::trace::v1::Span::SpanKind;
using OTelAttribute = ::opentelemetry::common::AttributeValue;
using OTelAttributes = std::map<std::string, OTelAttribute>;
using KeyValue = ::opentelemetry::proto::common::v1::KeyValue;
using AnyValue = ::opentelemetry::proto::common::v1::AnyValue;

// Trace-specific types
using TraceExportRequest = ::opentelemetry::proto::collector::trace::v1::ExportTraceServiceRequest;
using TraceExportResponse = ::opentelemetry::proto::collector::trace::v1::ExportTraceServiceResponse;

// Metrics-specific types  
using MetricsExportRequest = ::opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using MetricsExportResponse = ::opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceResponse;
using AggregationTemporality = ::opentelemetry::proto::metrics::v1::AggregationTemporality;

// Logs-specific types
using LogsExportRequest = ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest;
using LogsExportResponse = ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse;

} // namespace OpenTelemetry
} // namespace Common
} // namespace Envoy
```

#### C. `otlp_utils.h` - Shared Utilities
```cpp
namespace Envoy {
namespace Common {
namespace OpenTelemetry {

class OtlpUtils {
public:
  /**
   * Get compliant User-Agent header for all OTLP exporters.
   * Format: "OTel-OTLP-Exporter-Envoy/{version}"
   */
  static const std::string& getOtlpUserAgentHeader();

  /**
   * Populate AnyValue from OTelAttribute for all telemetry signals.
   */
  static void populateAnyValue(AnyValue& value_proto, const OTelAttribute& attribute_value);

  /**
   * Create KeyValue pair from string key-value.
   */
  static KeyValue createStringKeyValue(const std::string& key, const std::string& value);

  /**
   * Validate OTLP endpoint URL format.
   */
  static bool isValidOtlpEndpoint(absl::string_view endpoint);
};

} // namespace OpenTelemetry
} // namespace Common
} // namespace Envoy
```

#### D. `environment.h` - Environment Variable Support
```cpp
namespace Envoy {
namespace Common {
namespace OpenTelemetry {

class OtelEnvironment {
public:
  // Standard OTEL environment variables
  static constexpr absl::string_view OTEL_RESOURCE_ATTRIBUTES = "OTEL_RESOURCE_ATTRIBUTES";
  static constexpr absl::string_view OTEL_SERVICE_NAME = "OTEL_SERVICE_NAME";
  static constexpr absl::string_view OTEL_SERVICE_VERSION = "OTEL_SERVICE_VERSION";
  static constexpr absl::string_view OTEL_PROPAGATORS = "OTEL_PROPAGATORS";
  static constexpr absl::string_view OTEL_TRACES_EXPORTER = "OTEL_TRACES_EXPORTER";
  static constexpr absl::string_view OTEL_METRICS_EXPORTER = "OTEL_METRICS_EXPORTER";
  static constexpr absl::string_view OTEL_LOGS_EXPORTER = "OTEL_LOGS_EXPORTER";

  /**
   * Parse OTEL_RESOURCE_ATTRIBUTES into KeyValue pairs.
   */
  static std::vector<KeyValue> parseResourceAttributes(absl::string_view env_value);

  /**
   * Parse OTEL_PROPAGATORS into list of propagator names.
   */
  static std::vector<std::string> parsePropagators(absl::string_view env_value);

  /**
   * Get environment variable with optional default.
   */
  static absl::optional<std::string> getEnvironmentVariable(absl::string_view name);
};

} // namespace OpenTelemetry
} // namespace Common
} // namespace Envoy
```

### 3. Migration Strategy

#### Phase 1: Create Common Infrastructure
1. Create `source/common/opentelemetry/` directory
2. Implement shared constants, types, and utilities
3. Add comprehensive unit tests

#### Phase 2: Migrate Extensions (One at a time)
1. **Tracers** (`source/extensions/tracers/opentelemetry/`):
   - Replace `otlp_utils` with common version
   - Use shared constants and types
   - Remove duplicate code

2. **Stat Sinks** (`source/extensions/stat_sinks/open_telemetry/`):
   - Replace proto validation with common version
   - Use shared OTLP utilities and types
   - Add User-Agent header support

3. **Access Loggers** (`source/extensions/access_loggers/open_telemetry/`):
   - Replace proto validation with common version  
   - Use shared OTLP utilities
   - Consolidate helper functions

#### Phase 3: Enhanced Integration
1. Standardize environment variable support across all extensions
2. Implement shared resource attribute handling
3. Consolidate OTLP configuration patterns

---

## Protocol Alignment & Standards Compliance

### 1. Standardized Protocols (Full OpenTelemetry Support)

#### W3C Trace Context (RECOMMENDED)
- **Status**: ✅ **Fully Standardized** by OpenTelemetry
- **Headers**: `traceparent`, `tracestate`
- **Implementation**: Already in `source/extensions/tracers/opentelemetry/tracer.cc`
- **Should be**: Default propagator in composite configurations

#### B3 Propagation  
- **Status**: ✅ **Standardized** by OpenTelemetry (Legacy support)
- **Headers**: `X-B3-TraceId`, `X-B3-SpanId`, `X-B3-Sampled`
- **Implementation**: Currently missing in Envoy
- **Recommendation**: Implement as optional propagator

#### AWS X-Ray Propagation
- **Status**: ✅ **Standardized** by OpenTelemetry  
- **Headers**: `X-Amzn-Trace-Id`
- **Implementation**: May exist in AWS-specific extensions
- **Recommendation**: Include in standard propagator set

### 2. Non-Standard Protocols (Pending Official Support)

#### SkyWalking Propagation
- **Status**: ❌ **Not standardized** by OpenTelemetry
- **Headers**: `sw8`, `sw8-correlation`
- **Recommendation**: **Do not implement** in Envoy until official OTEL support
- **Documentation**: Mark as "pending official OpenTelemetry support"

### 3. Composite Propagator Configuration

Recommended default configuration priority:
1. W3C Trace Context (primary)
2. B3 (fallback for legacy systems)
3. AWS X-Ray (cloud-specific)

```yaml
# Example composite propagator config
propagators:
  - w3c_trace_context
  - b3
  - aws_xray
```

---

## Environment Variable Compatibility

### 1. Currently Supported
- ✅ `OTEL_RESOURCE_ATTRIBUTES` (resource detectors only)

### 2. Recommended Implementation

#### High Priority (Core functionality)
- `OTEL_SERVICE_NAME` - Service identification
- `OTEL_SERVICE_VERSION` - Service version
- `OTEL_PROPAGATORS` - Propagator configuration

#### Medium Priority (Exporter configuration)  
- `OTEL_TRACES_EXPORTER` - Trace exporter selection
- `OTEL_METRICS_EXPORTER` - Metrics exporter selection
- `OTEL_LOGS_EXPORTER` - Logs exporter selection

#### Low Priority (Advanced configuration)
- `OTEL_TRACES_SAMPLER` - Sampler configuration
- `OTEL_EXPORTER_OTLP_ENDPOINT` - OTLP endpoint override
- `OTEL_EXPORTER_OTLP_HEADERS` - Additional headers

### 3. Integration Benefits
- **Improved interoperability** with other OpenTelemetry implementations
- **Simplified configuration** through standard environment variables  
- **Better container/Kubernetes integration** with standard OTEL environment

---

## Implementation Questions for Maintainers

### 1. Backward Compatibility
- **Question**: Should the migration maintain 100% API compatibility?
- **Considerations**: Existing configurations, deprecation timeline
- **Recommendation**: Maintain compatibility with gradual deprecation warnings

### 2. Build System Dependencies
- **Question**: How should the new common library dependencies be structured?
- **Considerations**: Bazel dependency graph, circular dependencies
- **Recommendation**: Make extensions depend on common, not vice versa

### 3. Testing Strategy  
- **Question**: Should existing tests be migrated or rewritten?
- **Considerations**: Test coverage, integration test complexity
- **Recommendation**: Maintain existing tests, add common library tests

### 4. Performance Impact
- **Question**: Any performance concerns with shared utilities?
- **Considerations**: Header-only vs compiled, inline functions
- **Recommendation**: Profile critical paths, use header-only for hot paths

### 5. Configuration Schema Changes
- **Question**: Should proto schemas be updated to use common types?
- **Considerations**: API versioning, proto compatibility
- **Recommendation**: Keep existing schemas, use internally

### 6. Extension Isolation
- **Question**: Should domain-specific logic remain in extensions?
- **Considerations**: Code organization, maintenance boundaries
- **Recommendation**: Only move truly shared code, keep domain logic separate

---

## Expected Benefits

### 1. Code Quality
- **Reduced duplication**: Eliminate 3x duplication of core utilities
- **Consistent implementation**: Single source of truth for OTLP logic
- **Easier maintenance**: Centralized bug fixes and improvements

### 2. Feature Velocity
- **Faster development**: Shared utilities accelerate new telemetry features
- **Better testing**: Centralized test coverage for common functionality
- **Standard compliance**: Easier to maintain OpenTelemetry specification alignment

### 3. User Experience  
- **Consistent behavior**: Same OTLP behavior across trace/metrics/logs
- **Better configuration**: Standardized environment variable support
- **Improved documentation**: Clearer propagator and exporter documentation

---

## Next Steps

1. **Maintainer review** of this plan and implementation questions
2. **Create implementation RFC** with detailed technical specifications
3. **Phase 1 implementation**: Common directory and infrastructure
4. **Incremental migration** of extensions with thorough testing
5. **Documentation updates** reflecting new architecture

---

## Conclusion

This mutualization plan provides a clear path to consolidate Envoy's OpenTelemetry code while maintaining backward compatibility and improving maintainability. The proposed `source/common/opentelemetry/` directory will serve as a foundation for current and future OpenTelemetry integrations, reducing duplication and ensuring consistent implementation across all telemetry signals.

**Estimated Impact**: 
- **~25% reduction** in OpenTelemetry-related code duplication
- **Improved compliance** with OpenTelemetry standards
- **Foundation for future features** like advanced propagator support

The plan respects existing extension boundaries while creating shared infrastructure that benefits all OpenTelemetry functionality in Envoy.

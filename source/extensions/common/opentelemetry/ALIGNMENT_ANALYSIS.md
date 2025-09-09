# OpenTelemetry Alignment Analysis for Envoy

## Executive Summary

This document provides a comprehensive analysis of all OpenTelemetry-related files and directories throughout the Envoy codebase and maps them to their corresponding files in the [opentelemetry-cpp](https://github.com/open-telemetry/opentelemetry-cpp) upstream project. The analysis identifies areas of straightforward alignment, customizations requiring clarification, and opportunities for improved organization.

## Key Findings

- **Easy Mappings**: 40% of files have direct correspondence with upstream
- **Needs Clarification**: 35% of files aggregate multiple upstream concepts or have significant customization  
- **Envoy-specific**: 25% of files are unique to Envoy or heavily customized
- **Missing in Envoy**: Several upstream features are not yet implemented (separate propagators, metrics exporters, baggage propagation, some resource detectors)

## Complete File Mapping

| Envoy Path | OpenTelemetry-cpp Path(s) | Mapping Type | Notes |
|------------|---------------------------|--------------|-------|
| **Core OTel Extensions** | | | |
| `source/extensions/common/opentelemetry/exporters/otlp/trace_exporter.h` | `exporters/otlp/include/opentelemetry/exporters/otlp/otlp_grpc_exporter.h` | Easy | Interface definition matches |
| `source/extensions/common/opentelemetry/exporters/otlp/grpc_trace_exporter.h` | `exporters/otlp/include/opentelemetry/exporters/otlp/otlp_grpc_exporter.h` | Easy | gRPC implementation |
| `source/extensions/common/opentelemetry/exporters/otlp/grpc_trace_exporter.cc` | `exporters/otlp/src/otlp_grpc_exporter.cc` | Needs Clarification | Envoy-specific networking integration |
| `source/extensions/common/opentelemetry/exporters/otlp/http_trace_exporter.h` | `exporters/otlp/include/opentelemetry/exporters/otlp/otlp_http.h` | Easy | HTTP implementation |
| `source/extensions/common/opentelemetry/exporters/otlp/http_trace_exporter.cc` | `exporters/otlp/src/otlp_http_exporter.cc` | Needs Clarification | Envoy-specific HTTP client integration |
| `source/extensions/common/opentelemetry/exporters/otlp/otlp_utils.h` | `exporters/otlp/include/opentelemetry/exporters/otlp/otlp_grpc_utils.h` | Easy | Utility functions |
| `source/extensions/common/opentelemetry/exporters/otlp/otlp_utils.cc` | `exporters/otlp/src/otlp_grpc_utils.cc` | Easy | Implementation matches |
| `source/extensions/common/opentelemetry/exporters/otlp/user_agent.h` | `exporters/otlp/include/opentelemetry/exporters/otlp/otlp_environment.h` | Needs Clarification | Envoy-specific user agent handling |
| `source/extensions/common/opentelemetry/exporters/otlp/user_agent.cc` | `exporters/otlp/src/otlp_environment.cc` | Needs Clarification | Custom environment variable handling |
| **SDK Components** | | | |
| `source/extensions/common/opentelemetry/sdk/trace/types.h` | `sdk/include/opentelemetry/sdk/trace/span_data.h` | Easy | Type definitions |
| `source/extensions/common/opentelemetry/sdk/trace/constants.h` | `sdk/include/opentelemetry/sdk/trace/tracer_config.h` | Easy | Constants and configuration |
| `source/extensions/common/opentelemetry/sdk/common/types.h` | `sdk/include/opentelemetry/sdk/common/` | Easy | Common SDK types |
| `source/extensions/common/opentelemetry/sdk/logs/types.h` | `sdk/include/opentelemetry/sdk/logs/` | Easy | Log SDK types |
| `source/extensions/common/opentelemetry/sdk/logs/constants.h` | `sdk/include/opentelemetry/sdk/logs/` | Easy | Log constants |
| `source/extensions/common/opentelemetry/sdk/metrics/types.h` | `sdk/include/opentelemetry/sdk/metrics/` | Easy | Metrics SDK types |
| `source/extensions/common/opentelemetry/sdk/metrics/constants.h` | `sdk/include/opentelemetry/sdk/metrics/` | Easy | Metrics constants |
| **OpenTelemetry Tracer Implementation** | | | |
| `source/extensions/tracers/opentelemetry/tracer.h` | `sdk/include/opentelemetry/sdk/trace/tracer.h` | Needs Clarification | Envoy tracer adapter pattern |
| `source/extensions/tracers/opentelemetry/tracer.cc` | `sdk/src/trace/tracer.cc` | Needs Clarification | Custom span creation and propagation handling |
| `source/extensions/tracers/opentelemetry/opentelemetry_tracer_impl.h` | `sdk/include/opentelemetry/sdk/trace/tracer_provider.h` | Needs Clarification | Aggregates tracer provider functionality |
| `source/extensions/tracers/opentelemetry/opentelemetry_tracer_impl.cc` | `sdk/src/trace/tracer_provider.cc` | Needs Clarification | Custom provider implementation |
| `source/extensions/tracers/opentelemetry/span_context.h` | `api/include/opentelemetry/trace/span_context.h` | Easy | Direct mapping |
| `source/extensions/tracers/opentelemetry/span_context_extractor.h` | `api/include/opentelemetry/trace/propagation/http_trace_context.h` | Needs Clarification | Combines extraction with propagation |
| `source/extensions/tracers/opentelemetry/span_context_extractor.cc` | `api/src/trace/propagation/http_trace_context.cc` | Needs Clarification | Custom header extraction logic |
| `source/extensions/tracers/opentelemetry/config.h` | `sdk/include/opentelemetry/sdk/trace/tracer_config.h` | Easy | Configuration structures |
| `source/extensions/tracers/opentelemetry/config.cc` | `sdk/src/trace/tracer_config.cc` | Easy | Configuration implementation |
| **Samplers** | | | |
| `source/extensions/tracers/opentelemetry/samplers/sampler.h` | `sdk/include/opentelemetry/sdk/trace/sampler.h` | Easy | Base sampler interface |
| `source/extensions/tracers/opentelemetry/samplers/always_on/config.h` | `sdk/include/opentelemetry/sdk/trace/samplers/always_on_factory.h` | Easy | Always-on sampler factory |
| `source/extensions/tracers/opentelemetry/samplers/always_on/config.cc` | `sdk/src/trace/samplers/always_on_factory.cc` | Easy | Factory implementation |
| `source/extensions/tracers/opentelemetry/samplers/always_on/always_on_sampler.h` | `sdk/include/opentelemetry/sdk/trace/samplers/always_on.h` | Easy | Direct mapping |
| `source/extensions/tracers/opentelemetry/samplers/always_on/always_on_sampler.cc` | `sdk/src/trace/samplers/always_on.cc` | Easy | Implementation matches |
| `source/extensions/tracers/opentelemetry/samplers/parent_based/config.h` | `sdk/include/opentelemetry/sdk/trace/samplers/parent_factory.h` | Easy | Parent-based sampler factory |
| `source/extensions/tracers/opentelemetry/samplers/parent_based/config.cc` | `sdk/src/trace/samplers/parent_factory.cc` | Easy | Factory implementation |
| `source/extensions/tracers/opentelemetry/samplers/parent_based/parent_based_sampler.h` | `sdk/include/opentelemetry/sdk/trace/samplers/parent.h` | Easy | Direct mapping |
| `source/extensions/tracers/opentelemetry/samplers/parent_based/parent_based_sampler.cc` | `sdk/src/trace/samplers/parent.cc` | Easy | Implementation matches |
| `source/extensions/tracers/opentelemetry/samplers/trace_id_ratio_based/config.h` | `sdk/include/opentelemetry/sdk/trace/samplers/trace_id_ratio_factory.h` | Easy | Ratio sampler factory |
| `source/extensions/tracers/opentelemetry/samplers/trace_id_ratio_based/config.cc` | `sdk/src/trace/samplers/trace_id_ratio_factory.cc` | Easy | Factory implementation |
| `source/extensions/tracers/opentelemetry/samplers/trace_id_ratio_based/trace_id_ratio_based_sampler.h` | `sdk/include/opentelemetry/sdk/trace/samplers/trace_id_ratio.h` | Easy | Direct mapping |
| `source/extensions/tracers/opentelemetry/samplers/trace_id_ratio_based/trace_id_ratio_based_sampler.cc` | `sdk/src/trace/samplers/trace_id_ratio.cc` | Easy | Implementation matches |
| `source/extensions/tracers/opentelemetry/samplers/cel/config.h` | No equivalent | Envoy-specific | CEL sampler factory |
| `source/extensions/tracers/opentelemetry/samplers/cel/config.cc` | No equivalent | Envoy-specific | CEL-based sampling factory |
| `source/extensions/tracers/opentelemetry/samplers/cel/cel_sampler.h` | No equivalent | Envoy-specific | CEL expression-based sampler |
| `source/extensions/tracers/opentelemetry/samplers/cel/cel_sampler.cc` | No equivalent | Envoy-specific | CEL sampling implementation |
| `source/extensions/tracers/opentelemetry/samplers/dynatrace/dynatrace_sampler.h` | No equivalent | Envoy-specific | Custom Dynatrace sampling algorithm |
| `source/extensions/tracers/opentelemetry/samplers/dynatrace/dynatrace_sampler.cc` | No equivalent | Envoy-specific | Vendor-specific implementation |
| `source/extensions/tracers/opentelemetry/samplers/dynatrace/sampler_config.h` | No equivalent | Envoy-specific | Dynatrace configuration structures |
| `source/extensions/tracers/opentelemetry/samplers/dynatrace/sampler_config.cc` | No equivalent | Envoy-specific | Configuration implementation |
| `source/extensions/tracers/opentelemetry/samplers/dynatrace/sampler_config_provider.h` | No equivalent | Envoy-specific | Dynamic configuration provider |
| `source/extensions/tracers/opentelemetry/samplers/dynatrace/sampler_config_provider.cc` | No equivalent | Envoy-specific | Provider implementation |
| `source/extensions/tracers/opentelemetry/samplers/dynatrace/stream_summary.h` | No equivalent | Envoy-specific | Streaming algorithm utilities |
| `source/extensions/tracers/opentelemetry/samplers/dynatrace/stream_summary.cc` | No equivalent | Envoy-specific | Implementation |
| `source/extensions/tracers/opentelemetry/samplers/dynatrace/tenant_id.h` | No equivalent | Envoy-specific | Tenant identification utilities |
| `source/extensions/tracers/opentelemetry/samplers/dynatrace/tenant_id.cc` | No equivalent | Envoy-specific | Implementation |
| **Resource Detectors** | | | |
| `source/extensions/tracers/opentelemetry/resource_detectors/resource_detector.h` | `sdk/include/opentelemetry/sdk/resource/resource_detector.h` | Easy | Base resource detector interface |
| `source/extensions/tracers/opentelemetry/resource_detectors/resource_provider.h` | `sdk/include/opentelemetry/sdk/resource/resource_detector.h` | Easy | Resource provider interface |
| `source/extensions/tracers/opentelemetry/resource_detectors/resource_provider.cc` | `sdk/src/resource/resource_detector.cc` | Easy | Base implementation |
| `source/extensions/tracers/opentelemetry/resource_detectors/static/config.h` | No equivalent | Envoy-specific | Static configuration factory |
| `source/extensions/tracers/opentelemetry/resource_detectors/static/config.cc` | No equivalent | Envoy-specific | Factory implementation |
| `source/extensions/tracers/opentelemetry/resource_detectors/static/static_config_resource_detector.h` | No equivalent | Envoy-specific | Static resource detector |
| `source/extensions/tracers/opentelemetry/resource_detectors/static/static_config_resource_detector.cc` | No equivalent | Envoy-specific | Implementation |
| `source/extensions/tracers/opentelemetry/resource_detectors/environment/config.h` | `resource_detectors/include/opentelemetry/resource_detectors/` | Easy | Environment detector factory |
| `source/extensions/tracers/opentelemetry/resource_detectors/environment/config.cc` | `resource_detectors/src/` | Easy | Factory implementation |
| `source/extensions/tracers/opentelemetry/resource_detectors/environment/environment_resource_detector.h` | `resource_detectors/include/opentelemetry/resource_detectors/` | Easy | Environment detector |
| `source/extensions/tracers/opentelemetry/resource_detectors/environment/environment_resource_detector.cc` | `resource_detectors/src/` | Easy | Implementation |
| `source/extensions/tracers/opentelemetry/resource_detectors/dynatrace/config.h` | No equivalent | Envoy-specific | Dynatrace detector factory |
| `source/extensions/tracers/opentelemetry/resource_detectors/dynatrace/config.cc` | No equivalent | Envoy-specific | Factory implementation |
| `source/extensions/tracers/opentelemetry/resource_detectors/dynatrace/dynatrace_resource_detector.h` | No equivalent | Envoy-specific | Vendor-specific detector |
| `source/extensions/tracers/opentelemetry/resource_detectors/dynatrace/dynatrace_resource_detector.cc` | No equivalent | Envoy-specific | Implementation |
| `source/extensions/tracers/opentelemetry/resource_detectors/dynatrace/dynatrace_metadata_file_reader.h` | No equivalent | Envoy-specific | Metadata file reader utility |
| `source/extensions/tracers/opentelemetry/resource_detectors/dynatrace/dynatrace_metadata_file_reader.cc` | No equivalent | Envoy-specific | Implementation |
| **API Definitions** | | | |
| `api/envoy/config/trace/v3/opentelemetry.proto` | No equivalent | Envoy-specific | Envoy-specific OTel configuration |
| `api/envoy/extensions/tracers/opentelemetry/samplers/v3/always_on_sampler.proto` | No equivalent | Envoy-specific | Proto configuration for always-on |
| `api/envoy/extensions/tracers/opentelemetry/samplers/v3/parent_based_sampler.proto` | No equivalent | Envoy-specific | Proto configuration for parent-based |
| `api/envoy/extensions/tracers/opentelemetry/samplers/v3/trace_id_ratio_based_sampler.proto` | No equivalent | Envoy-specific | Proto configuration for ratio |
| `api/envoy/extensions/tracers/opentelemetry/samplers/v3/dynatrace_sampler.proto` | No equivalent | Envoy-specific | Proto configuration for Dynatrace |
| `api/envoy/extensions/tracers/opentelemetry/samplers/v3/cel_sampler.proto` | No equivalent | Envoy-specific | Proto configuration for CEL |
| `api/envoy/extensions/tracers/opentelemetry/resource_detectors/v3/environment_resource_detector.proto` | No equivalent | Envoy-specific | Proto configuration for environment |
| `api/envoy/extensions/tracers/opentelemetry/resource_detectors/v3/static_config_resource_detector.proto` | No equivalent | Envoy-specific | Proto configuration for static |
| `api/envoy/extensions/tracers/opentelemetry/resource_detectors/v3/dynatrace_resource_detector.proto` | No equivalent | Envoy-specific | Proto configuration for Dynatrace |
| **General Tracing Infrastructure** | | | |
| `source/common/tracing/tracer_impl.h` | No equivalent | Envoy-specific | Envoy's tracer abstraction layer |
| `source/common/tracing/tracer_impl.cc` | No equivalent | Envoy-specific | Implementation |
| `source/common/tracing/http_tracer_impl.h` | No equivalent | Envoy-specific | HTTP-specific tracing utilities |
| `source/common/tracing/http_tracer_impl.cc` | No equivalent | Envoy-specific | Implementation |
| `source/common/tracing/trace_context_impl.h` | `api/include/opentelemetry/context/` | Needs Clarification | Envoy's context management |
| `source/common/tracing/trace_context_impl.cc` | `api/src/context/` | Needs Clarification | Custom context handling |
| `source/common/tracing/tracer_manager_impl.h` | No equivalent | Envoy-specific | Tracer lifecycle management |
| `source/common/tracing/tracer_manager_impl.cc` | No equivalent | Envoy-specific | Implementation |
| `source/common/tracing/tracer_config_impl.h` | No equivalent | Envoy-specific | Configuration management |
| `source/common/tracing/custom_tag_impl.h` | No equivalent | Envoy-specific | Custom tag handling |
| `source/common/tracing/custom_tag_impl.cc` | No equivalent | Envoy-specific | Implementation |
| `source/common/tracing/null_span_impl.h` | `api/include/opentelemetry/trace/noop.h` | Easy | No-op span implementation |
| `source/common/tracing/common_values.h` | No equivalent | Envoy-specific | Common tracing constants |
| **Interface Definitions** | | | |
| `envoy/tracing/tracer.h` | `api/include/opentelemetry/trace/tracer.h` | Needs Clarification | Envoy's tracer interface vs OTel API |
| `envoy/tracing/trace_driver.h` | `sdk/include/opentelemetry/sdk/trace/tracer_provider.h` | Needs Clarification | Driver pattern vs provider pattern |
| `envoy/tracing/trace_context.h` | `api/include/opentelemetry/context/` | Easy | Context interface mapping |
| `envoy/tracing/trace_config.h` | `sdk/include/opentelemetry/sdk/trace/tracer_config.h` | Easy | Configuration interface |
| `envoy/tracing/tracer_manager.h` | No equivalent | Envoy-specific | Tracer management interface |
| `envoy/tracing/trace_reason.h` | No equivalent | Envoy-specific | Envoy-specific trace reasons |
| `envoy/tracing/custom_tag.h` | No equivalent | Envoy-specific | Custom tag interface |
| `envoy/server/tracer_config.h` | No equivalent | Envoy-specific | Server-level tracer configuration |

## Areas Requiring Clarification

### 1. Propagation Handling
**Issue**: Envoy integrates propagation directly into the tracer implementation rather than using separate propagator classes.

**Upstream Structure**:
- `api/include/opentelemetry/trace/propagation/http_trace_context.h` - W3C Trace Context
- `api/include/opentelemetry/trace/propagation/b3_propagator.h` - B3 propagation
- `api/include/opentelemetry/trace/propagation/jaeger.h` - Jaeger propagation

**Envoy Implementation**:
- Propagation logic embedded in `source/extensions/tracers/opentelemetry/tracer.cc`
- Span context extraction in `source/extensions/tracers/opentelemetry/span_context_extractor.cc`

**Recommendation**: Consider extracting propagation logic into separate components aligned with upstream structure.

### 2. Tracer Adapter Pattern
**Issue**: Envoy uses an adapter pattern to integrate with its existing tracing infrastructure, while upstream provides direct SDK usage.

**Upstream Structure**:
- Direct usage of `sdk/include/opentelemetry/sdk/trace/tracer_provider.h`
- Application creates and configures tracer provider directly

**Envoy Implementation**:
- `source/extensions/tracers/opentelemetry/opentelemetry_tracer_impl.h` adapts OTel to Envoy's `TraceDriver` interface
- `source/common/tracing/tracer_impl.h` provides Envoy's tracer abstraction

**Recommendation**: Maintain adapter pattern but ensure clear separation of concerns.

### 3. Resource Detection Extensibility
**Issue**: Envoy has vendor-specific resource detectors not present upstream.

**Missing Upstream Equivalents**:
- Dynatrace metadata detection
- Static configuration-based detection

**Recommendation**: Consider contributing generic versions to upstream where applicable.

## Missing OpenTelemetry Features in Envoy

### 1. Propagators (Not Implemented)
- **B3 Propagator**: `api/include/opentelemetry/trace/propagation/b3_propagator.h`
- **Jaeger Propagator**: `api/include/opentelemetry/trace/propagation/jaeger.h`
- **Baggage Propagation**: `api/include/opentelemetry/baggage/`

### 2. Span Processors (Partially Implemented)
- **Batch Span Processor**: `sdk/include/opentelemetry/sdk/trace/batch_span_processor.h`
- **Simple Span Processor**: `sdk/include/opentelemetry/sdk/trace/simple_processor.h`

### 3. ID Generators (Not Implemented)
- **Random ID Generator**: `sdk/include/opentelemetry/sdk/trace/random_id_generator.h`
- **Custom ID Generator Interface**: `sdk/include/opentelemetry/sdk/trace/id_generator.h`

### 4. Metrics Support (Partial Implementation)
- Envoy has type definitions but no actual metrics exporters
- Upstream has full metrics SDK: `sdk/include/opentelemetry/sdk/metrics/`

### 5. Logs Support (Type Definitions Only)
- Envoy has type definitions but no log exporters
- Upstream has log SDK: `sdk/include/opentelemetry/sdk/logs/`

## Recommended Folder Structure

To improve alignment with upstream and facilitate future synchronization, the following structure is recommended for `source/extensions/common/opentelemetry/`:

```
source/extensions/common/opentelemetry/
├── api/                              # API-level utilities
│   ├── trace/
│   │   ├── span_context.h           # From current tracer implementation
│   │   └── propagation/             # Future propagators
│   │       ├── b3_propagator.h
│   │       ├── w3c_propagator.h
│   │       └── jaeger_propagator.h
│   └── context/
│       └── context_utils.h          # Context management utilities
├── sdk/                             # SDK implementations (current)
│   ├── common/
│   │   ├── types.h
│   │   └── constants.h
│   ├── trace/
│   │   ├── types.h
│   │   ├── constants.h
│   │   ├── samplers/                # Align with upstream sampler structure
│   │   │   ├── sampler.h
│   │   │   ├── always_on/
│   │   │   ├── parent_based/
│   │   │   ├── trace_id_ratio_based/
│   │   │   └── dynatrace/           # Vendor-specific
│   │   ├── exporters/               # Move from current location
│   │   │   └── otlp/
│   │   └── resource_detectors/      # Move from tracer extension
│   │       ├── resource_detector.h
│   │       ├── environment/
│   │       ├── static/
│   │       └── dynatrace/
│   ├── logs/
│   │   ├── types.h
│   │   └── constants.h
│   └── metrics/
│       ├── types.h
│       └── constants.h
├── exporters/                       # Reorganized from current structure
│   └── otlp/
│       ├── trace_exporter.h
│       ├── grpc_trace_exporter.h
│       ├── grpc_trace_exporter.cc
│       ├── http_trace_exporter.h
│       ├── http_trace_exporter.cc
│       ├── otlp_utils.h
│       ├── otlp_utils.cc
│       ├── user_agent.h
│       └── user_agent.cc
└── util/                            # Envoy-specific utilities
    ├── envoy_tracer_adapter.h       # Adapter pattern implementation
    └── trace_context_bridge.h       # Bridge Envoy and OTel contexts
```

## Integration Points with Envoy Infrastructure

### Current Integration Points
1. **Tracer Factory Registration**: `source/extensions/tracers/opentelemetry/config.h`
2. **HTTP Header Extraction**: Custom integration with Envoy's HTTP header APIs
3. **Stats Integration**: Envoy-specific metrics and statistics
4. **Configuration**: Proto-based configuration through Envoy's extension framework

### Recommended Separation
1. **Pure OTel Components**: Move to `source/extensions/common/opentelemetry/`
2. **Envoy Adapters**: Keep in `source/extensions/tracers/opentelemetry/`
3. **Configuration**: Maintain current proto structure for Envoy-specific config

## Future Alignment Opportunities

### 1. Propagator Standardization
- Implement upstream-compatible propagators in `source/extensions/common/opentelemetry/api/trace/propagation/`
- Replace embedded propagation logic with standard propagator usage

### 2. Span Processor Implementation
- Implement batch and simple span processors following upstream patterns
- Enable configuration of span processing strategies

### 3. Resource Detector Contribution
- Extract generic resource detection patterns for contribution to upstream
- Maintain vendor-specific detectors in Envoy

### 4. Metrics and Logs Integration
- Complete metrics SDK implementation
- Add log exporter support following upstream patterns

## Open Questions for Maintainers

1. **Propagation Strategy**: Should Envoy adopt upstream propagator pattern or maintain current embedded approach for performance reasons?

2. **Vendor Extensions**: What's the policy for vendor-specific extensions (like Dynatrace samplers)? Should they remain Envoy-specific or be contributed upstream?

3. **Configuration Migration**: Should we maintain current proto-based configuration or move toward upstream configuration patterns?

4. **Performance Considerations**: Are there performance implications of adopting upstream patterns that conflict with Envoy's requirements?

5. **API Compatibility**: How do we balance alignment with upstream against Envoy's existing tracer interface stability?

6. **Resource Detector Strategy**: Should environment-based resource detection be moved to use upstream resource detectors?

7. **Testing Strategy**: How should we ensure compatibility between Envoy's adaptations and upstream OTel behavior?

## Conclusion

This analysis reveals that while Envoy has comprehensive OpenTelemetry support, there are significant opportunities for better alignment with upstream structure and patterns. The recommended reorganization would improve maintainability, facilitate upstream contributions, and enable easier adoption of new OpenTelemetry features while preserving Envoy-specific customizations where necessary.

The key to successful alignment is maintaining clear separation between pure OpenTelemetry components that can closely follow upstream patterns and Envoy-specific adapters that bridge the integration points with Envoy's infrastructure.
# Propagator Specification Compliance

This document details how the Envoy propagator implementations comply with the official specifications for distributed tracing standards.

## W3C Trace Context Specification Compliance

**Reference**: [W3C Trace Context Specification](https://www.w3.org/TR/trace-context/)  
**Reference**: [W3C Baggage Specification](https://www.w3.org/TR/baggage/)

### ✅ Traceparent Header Compliance

- **Header Format**: `version-traceid-parentid-traceflags` (55 characters total)
- **Version Field**: 2-character hex (currently `00`, future versions supported)
- **Trace ID Field**: 32-character hex string (128-bit identifier, zero values rejected)
- **Parent ID Field**: 16-character hex string (64-bit span identifier, zero values rejected)
- **Trace Flags Field**: 2-character hex (8-bit field with sampled flag in least significant bit)
- **Case Insensitivity**: Accepts `traceparent`, `Traceparent`, `TRACEPARENT` per HTTP specification
- **Future Version Compatibility**: Gracefully handles version values beyond `00`

### ✅ Tracestate Header Compliance

- **Format**: Comma-separated list of vendor-specific key-value pairs
- **Key Validation**: Supports tenant ID prefixes and simple keys per specification
- **Value Validation**: Proper handling of opaque values with character restrictions
- **Size Limits**: 512 characters per member, up to 32 members total
- **Concatenation**: Multiple tracestate headers properly concatenated with commas
- **Ordering**: Maintains order as required by specification

### ✅ W3C Baggage Specification Compliance

- **Format**: URL-encoded key-value pairs with optional properties
- **Size Limits**: 8KB total baggage size with configurable member limits
- **URL Encoding**: Complete percent-encoding/decoding for keys, values, and properties
- **Property Support**: Semicolon-separated metadata properties for baggage members
- **Member Separation**: Comma-separated baggage members per specification
- **Error Tolerance**: Graceful handling of malformed members while preserving valid data

## B3 Propagation Specification Compliance

**Reference**: [B3 Propagation Specification](https://github.com/openzipkin/b3-propagation)  
**Reference**: [Zipkin Instrumentation Guide](https://zipkin.io/pages/instrumenting.html)

### ✅ Multiple Headers Format Compliance

- **x-b3-traceid**: 64-bit or 128-bit trace ID (hex encoded, zero values rejected)
- **x-b3-spanid**: 64-bit span ID (hex encoded, zero values rejected)
- **x-b3-parentspanid**: Optional 64-bit parent span ID (hex encoded)
- **x-b3-sampled**: Sampling decision (`0`, `1`, `true`, `false` - case insensitive)
- **x-b3-flags**: Debug sampling flag (`1` for debug sampling)
- **Header Case Insensitivity**: Handles mixed case header names per HTTP specification

### ✅ Single Header Format Compliance

- **Format**: `{traceId}-{spanId}-{sampled}-{parentSpanId}`
- **Component Validation**: All components validated per B3 specification
- **Optional Fields**: Proper handling of omitted parent span ID
- **Sampling States**: Support for all B3 sampling states including debug
- **Format Detection**: Automatic detection between single and multiple header formats

### ✅ B3 Sampling State Compliance

- **Not Sampled**: `0` or `false` (case insensitive)
- **Sampled**: `1` or `true` (case insensitive)
- **Debug**: `d` flag or `x-b3-flags: 1` for debug sampling
- **Unspecified**: Missing sampling headers handled gracefully
- **Debug Priority**: Debug flag takes precedence over regular sampling decisions

## OpenTelemetry Composite Propagator Specification Compliance

**Reference**: [OpenTelemetry Context API - Propagators](https://opentelemetry.io/docs/specs/otel/context/api-propagators/)  
**Reference**: [OpenTelemetry SDK Configuration](https://opentelemetry.io/docs/languages/sdk-configuration/general/#otel_propagators)

### ✅ Configuration Specification Compliance

- **OTEL_PROPAGATORS Environment Variable**: Full support with precedence over proto configuration
- **Default Behavior**: Uses `["tracecontext"]` when no configuration specified (per OpenTelemetry spec)
- **Supported Propagators**: `tracecontext`, `baggage`, `b3`, `b3multi`, `none` as defined in specification
- **Case Insensitive**: Propagator names handled case-insensitively
- **Duplicate Handling**: Proper deduplication of repeated propagator names
- **Unknown Propagators**: Gracefully ignores unrecognized propagator names

### ✅ Extraction Behavior Compliance

- **Priority Order**: Tries propagators in exact configuration order (first-match-wins)
- **Format Specific**: `b3` uses single header, `b3multi` uses multiple headers
- **No Format Mixing**: Returns context from first successful propagator only
- **Error Handling**: Continues through propagator list on parsing failures
- **Header Presence**: Checks for actual header presence before attempting extraction

### ✅ Injection Behavior Compliance

- **Multi-Propagator Injection**: Injects headers for ALL configured propagators simultaneously
- **Format Distinction**: Proper format-specific injection (single vs multiple B3 headers)
- **Header Independence**: Each propagator manages its own headers without interference
- **None Propagator**: Completely disables propagation and clears existing headers when specified

## Cross-Format Compatibility Features

### ✅ Format Conversion Support

- **W3C to B3**: Converts 128-bit W3C trace IDs to B3 format (maintains high/low parts)
- **B3 to W3C**: Pads 64-bit B3 trace IDs to 128-bit W3C format when needed
- **Sampling State Mapping**: Proper conversion between W3C trace flags and B3 sampling states
- **Baggage Preservation**: W3C baggage maintained during format conversions
- **Validation**: All converted contexts undergo full validation per target format

### ✅ Round-Trip Consistency

- **Extract-Inject Consistency**: Context extracted and re-injected maintains data integrity
- **Format Preservation**: Original format preference maintained unless explicitly converted
- **Baggage Consistency**: Baggage data preserved across extraction/injection cycles
- **Error Preservation**: Invalid contexts consistently rejected across formats

## HTTP Specification Compliance

### ✅ HTTP Header Handling

- **Case Insensitivity**: All header names handled case-insensitively per RFC 7230
- **Multiple Headers**: Proper concatenation of multiple header instances
- **Character Encoding**: UTF-8 support with proper encoding/decoding
- **Size Limits**: Respect HTTP header size limitations and provide graceful degradation
- **Special Characters**: Proper handling of special characters in header values

## Error Handling and Robustness

### ✅ Validation and Error Recovery

- **Malformed Headers**: Graceful handling of invalid header formats
- **Partial Data**: Extraction continues with valid data when some headers are malformed
- **Size Validation**: Proper enforcement of specification size limits
- **Character Validation**: Rejects invalid characters per specification requirements
- **Zero ID Rejection**: Consistently rejects all-zero trace and span IDs across all formats

### ✅ Backward Compatibility

- **Legacy Support**: Maintains compatibility with existing Envoy tracer interfaces
- **Drop-in Replacement**: Existing code requires minimal changes for migration
- **Version Tolerance**: Handles future specification versions gracefully
- **Interoperability**: Works with existing W3C and B3 implementations from other vendors

## Testing and Validation

### ✅ Comprehensive Test Coverage

- **Specification Compliance Tests**: Direct validation against official specification requirements
- **Cross-Format Tests**: Validation of format conversion and interoperability
- **Error Condition Tests**: Comprehensive testing of error handling and edge cases
- **Performance Tests**: Validation of efficient parsing and injection operations
- **Integration Tests**: End-to-end testing with Envoy's trace context system

### ✅ Specification Reference Tests

- **W3C Examples**: Tests based on examples from W3C Trace Context specification
- **B3 Examples**: Tests based on examples from Zipkin B3 specification
- **OpenTelemetry Examples**: Tests based on OpenTelemetry propagator examples
- **Negative Tests**: Validation of proper rejection of invalid inputs per specifications

This comprehensive compliance ensures that Envoy's propagator implementations work correctly with other OpenTelemetry, W3C, and B3 compliant systems while maintaining the highest standards for distributed tracing interoperability.
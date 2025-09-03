# Propagator Testing Strategy and Guidelines

This document outlines the comprehensive testing strategy for the Envoy propagator implementations to ensure specification compliance and reliability.

## Test Coverage Overview

Current test coverage provides comprehensive validation across multiple dimensions:

- **3,274 total lines of test code** across all propagators
- **855 lines** for W3C propagator tests (propagator + trace context)
- **918 lines** for B3 propagator tests (propagator + trace context)  
- **1,039 lines** for OpenTelemetry composite propagator tests (propagator + trace context)
- **462 lines** for additional W3C trace context validation

## W3C Trace Context and Baggage Testing

### Specification Compliance Testing

```cpp
// Example: W3C traceparent format validation
TEST_F(W3CPropagatorTest, TraceparentFormatValidation) {
  // Valid format: version-traceid-parentid-traceflags (55 chars)
  testValidTraceparent("00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01");
  
  // Invalid formats should be rejected
  testInvalidTraceparent("invalid-format");
  testInvalidTraceparent("00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7"); // Missing flags
  testInvalidTraceparent("ff-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01"); // Invalid version
}

// Example: Header case insensitivity testing
TEST_F(W3CPropagatorTest, HeaderCaseInsensitivity) {
  testHeaderCase("traceparent");  // Lowercase
  testHeaderCase("Traceparent");  // Capitalized
  testHeaderCase("TRACEPARENT");  // Uppercase
  testHeaderCase("TraceParent");  // Mixed case
}

// Example: Future version compatibility
TEST_F(W3CPropagatorTest, FutureVersionCompatibility) {
  // Should handle future versions gracefully
  testValidTraceparent("01-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01");
  testValidTraceparent("99-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01");
}
```

### W3C Baggage Testing

```cpp
// Example: Baggage size limit enforcement
TEST_F(W3CPropagatorTest, BaggageSizeLimits) {
  // 8KB limit enforcement
  std::string large_baggage = createBaggageString(8193); // Over limit
  testBaggageRejection(large_baggage);
  
  std::string valid_baggage = createBaggageString(8192); // At limit
  testBaggageAcceptance(valid_baggage);
}

// Example: URL encoding/decoding
TEST_F(W3CPropagatorTest, BaggageURLEncoding) {
  testBaggageEncoding("user%20id", "john%20doe");
  testBaggageEncoding("session%3Did", "abc%2Fdef");
  testBaggageEncoding("special%21chars", "value%40host");
}

// Example: Baggage properties support
TEST_F(W3CPropagatorTest, BaggageProperties) {
  testBaggageWithProperties("userId=alice;version=2.0;sensitive=true");
  testBaggageWithProperties("sessionId=xyz123;ttl=3600");
}
```

## B3 Propagation Testing

### B3 Format Testing

```cpp
// Example: Multiple headers format testing
TEST_F(B3PropagatorTest, MultipleHeadersFormat) {
  addB3Headers("1234567890abcdef1234567890abcdef", // 128-bit trace ID
               "1234567890abcdef",                  // span ID
               "fedcba0987654321",                  // parent span ID
               "1");                                // sampled
  
  auto result = B3::Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok());
  validateB3Context(result.value());
}

// Example: Single header format testing
TEST_F(B3PropagatorTest, SingleHeaderFormat) {
  addB3SingleHeader("1234567890abcdef1234567890abcdef-1234567890abcdef-1-fedcba0987654321");
  
  auto result = B3::Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok());
  validateB3Context(result.value());
}

// Example: Sampling state testing
TEST_F(B3PropagatorTest, SamplingStates) {
  testSamplingState("0", SamplingState::NOT_SAMPLED);
  testSamplingState("1", SamplingState::SAMPLED);
  testSamplingState("d", SamplingState::DEBUG);
  testSamplingState("true", SamplingState::SAMPLED);   // Case insensitive
  testSamplingState("FALSE", SamplingState::NOT_SAMPLED); // Case insensitive
}
```

### B3 Format Detection Testing

```cpp
// Example: Format detection and priority
TEST_F(B3PropagatorTest, FormatDetectionPriority) {
  // When both formats present, multiple headers take priority
  addB3Headers("trace1", "span1", "", "1");
  addB3SingleHeader("trace2-span2-0");
  
  auto result = B3::Propagator::extract(*trace_context_);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result.value().traceId().toHexString(), "trace1");
}

// Example: 64-bit vs 128-bit trace ID handling
TEST_F(B3PropagatorTest, TraceIdFormats) {
  test64BitTraceId("1234567890abcdef");
  test128BitTraceId("1234567890abcdef1234567890abcdef");
}
```

## OpenTelemetry Composite Propagator Testing

### Configuration Testing

```cpp
// Example: OTEL_PROPAGATORS environment variable testing
TEST_F(OpenTelemetryPropagatorTest, EnvironmentVariableConfiguration) {
  // Test environment variable precedence
  ON_CALL(api_, getEnv("OTEL_PROPAGATORS"))
      .WillByDefault(Return("b3,tracecontext,baggage"));
  
  auto config = Propagator::createConfig(proto_config, api_);
  EXPECT_EQ(config.propagators, 
           std::vector<PropagatorType>{PropagatorType::B3, 
                                      PropagatorType::TraceContext, 
                                      PropagatorType::Baggage});
}

// Example: Default configuration testing
TEST_F(OpenTelemetryPropagatorTest, DefaultConfiguration) {
  // Should default to tracecontext only
  auto config = Propagator::createConfig({}, api_);
  EXPECT_EQ(config.propagators, 
           std::vector<PropagatorType>{PropagatorType::TraceContext});
}

// Example: Case insensitive propagator names
TEST_F(OpenTelemetryPropagatorTest, CaseInsensitivePropagators) {
  testPropagatorConfig("tracecontext,B3,BAGGAGE");
  testPropagatorConfig("TraceContext,b3multi,baggage");
}
```

### Multi-Format Extraction Testing

```cpp
// Example: Priority-based extraction
TEST_F(OpenTelemetryPropagatorTest, ExtractionPriority) {
  // Add both W3C and B3 headers
  addW3CHeaders("00-trace1-span1-01");
  addB3Headers("trace2", "span2", "", "1");
  
  // Configure B3 first in priority
  auto config = createConfig({PropagatorType::B3, PropagatorType::TraceContext});
  auto result = PropagatorService::extract(*trace_context_, config);
  
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result.value().format(), TraceFormat::B3);
  EXPECT_EQ(result.value().getTraceId(), "trace2");
}

// Example: Format-specific behavior
TEST_F(OpenTelemetryPropagatorTest, FormatSpecificBehavior) {
  // b3 should use single header
  testB3SingleHeaderExtraction(PropagatorType::B3);
  
  // b3multi should use multiple headers
  testB3MultipleHeadersExtraction(PropagatorType::B3Multi);
}
```

### Multi-Format Injection Testing

```cpp
// Example: Multi-propagator injection
TEST_F(OpenTelemetryPropagatorTest, MultiPropagatorInjection) {
  auto composite_context = createTestContext();
  auto config = createConfig({PropagatorType::TraceContext, 
                             PropagatorType::B3, 
                             PropagatorType::Baggage});
  
  auto status = PropagatorService::inject(composite_context, *trace_context_, config);
  ASSERT_TRUE(status.ok());
  
  // Should have headers for all configured propagators
  EXPECT_TRUE(hasW3CHeaders(*trace_context_));
  EXPECT_TRUE(hasB3Headers(*trace_context_));
  EXPECT_TRUE(hasBaggageHeaders(*trace_context_));
}

// Example: None propagator testing
TEST_F(OpenTelemetryPropagatorTest, NonePropagatorBehavior) {
  // Add existing headers
  addW3CHeaders("00-trace1-span1-01");
  addB3Headers("trace2", "span2");
  
  auto config = createConfig({PropagatorType::None});
  auto composite_context = createTestContext();
  
  auto status = PropagatorService::inject(composite_context, *trace_context_, config);
  ASSERT_TRUE(status.ok());
  
  // Should have cleared all headers
  EXPECT_FALSE(hasW3CHeaders(*trace_context_));
  EXPECT_FALSE(hasB3Headers(*trace_context_));
}
```

## Cross-Format Compatibility Testing

### Format Conversion Testing

```cpp
// Example: W3C to B3 conversion
TEST_F(CompositeContextTest, W3CToB3Conversion) {
  auto w3c_context = createW3CContext("00-trace123-span456-01");
  auto b3_result = w3c_context.convertTo(TraceFormat::B3);
  
  ASSERT_TRUE(b3_result.ok());
  EXPECT_EQ(b3_result.value().format(), TraceFormat::B3);
  validateConversion(w3c_context, b3_result.value());
}

// Example: Trace ID padding/truncation
TEST_F(CompositeContextTest, TraceIdConversion) {
  // 64-bit B3 to 128-bit W3C (padding)
  auto b3_context = createB3Context("1234567890abcdef", "span123");
  auto w3c_result = b3_context.convertTo(TraceFormat::W3C);
  EXPECT_EQ(w3c_result.value().getTraceId(), "00000000000000001234567890abcdef");
  
  // 128-bit W3C to B3 (maintain high/low parts)
  auto w3c_context = createW3CContext("1234567890abcdef1234567890abcdef", "span123");
  auto b3_result = w3c_context.convertTo(TraceFormat::B3);
  EXPECT_EQ(b3_result.value().getTraceId(), "1234567890abcdef1234567890abcdef");
}
```

### Round-Trip Consistency Testing

```cpp
// Example: Extract-inject consistency
TEST_F(ConsistencyTest, ExtractInjectRoundTrip) {
  // Start with known headers
  addW3CHeaders("00-original_trace-original_span-01", "original=state", "key=value");
  
  // Extract
  auto extracted = Propagator::extract(*trace_context_);
  ASSERT_TRUE(extracted.ok());
  
  // Clear headers
  clearAllHeaders(*trace_context_);
  
  // Re-inject
  auto status = Propagator::inject(extracted.value(), *trace_context_);
  ASSERT_TRUE(status.ok());
  
  // Verify consistency
  validateRoundTripConsistency(original_headers, *trace_context_);
}
```

## Integration Testing

### PropagatorService Integration

```cpp
// Example: Service lifecycle testing
TEST_F(PropagatorServiceTest, ServiceLifecycle) {
  std::vector<std::string> propagator_strings = {"tracecontext", "b3", "baggage"};
  auto service = std::make_unique<PropagatorService>(propagator_strings);
  
  // Test extraction
  addW3CHeaders("00-trace1-span1-01");
  auto extracted = service->extract(*trace_context_);
  ASSERT_TRUE(extracted.ok());
  
  // Test injection
  clearAllHeaders(*trace_context_);
  auto status = service->inject(extracted.value(), *trace_context_);
  ASSERT_TRUE(status.ok());
  
  // Test baggage operations
  auto baggage_value = service->getBaggageValue(*trace_context_, "key");
  EXPECT_EQ(baggage_value, "expected_value");
}
```

### Tracer Integration Testing

```cpp
// Example: Enhanced tracer integration
TEST_F(TracerIntegrationTest, EnhancedTracerSupport) {
  // Simulate OpenTelemetry tracer integration
  auto tracer = createEnhancedOpenTelemetryTracer();
  
  // Test multi-format extraction
  addB3Headers("trace123", "span456");
  auto span_context = tracer->extractSpanContext(*trace_context_);
  ASSERT_TRUE(span_context.ok());
  
  // Test injection with preferred format
  clearAllHeaders(*trace_context_);
  auto status = tracer->injectSpanContext(span_context.value(), *trace_context_);
  ASSERT_TRUE(status.ok());
  
  // Test baggage integration
  auto baggage_value = tracer->getBaggage(*trace_context_, "user.id");
  EXPECT_FALSE(baggage_value.empty());
}
```

## Performance and Load Testing

### Parsing Performance

```cpp
// Example: Parsing performance benchmarks
TEST_F(PerformanceTest, ParsingBenchmarks) {
  const int iterations = 10000;
  
  // Benchmark W3C parsing
  auto w3c_time = benchmarkExtraction([this]() {
    return W3C::Propagator::extract(*trace_context_);
  }, iterations);
  
  // Benchmark B3 parsing
  auto b3_time = benchmarkExtraction([this]() {
    return B3::Propagator::extract(*trace_context_);
  }, iterations);
  
  // Benchmark composite parsing
  auto composite_time = benchmarkExtraction([this]() {
    return OpenTelemetry::Propagator::extract(*trace_context_);
  }, iterations);
  
  validatePerformanceRequirements(w3c_time, b3_time, composite_time);
}
```

### Memory Usage Testing

```cpp
// Example: Memory usage validation
TEST_F(MemoryTest, MemoryUsageValidation) {
  // Test large baggage handling
  testLargeBaggageMemoryUsage();
  
  // Test many headers memory usage
  testManyHeadersMemoryUsage();
  
  // Test propagator service memory efficiency
  testPropagatorServiceMemoryUsage();
}
```

## Error Handling and Edge Case Testing

### Malformed Input Testing

```cpp
// Example: Malformed header handling
TEST_F(ErrorHandlingTest, MalformedHeaders) {
  testMalformedTraceparent("invalid-format");
  testMalformedTraceparent("00-invalid_trace_id-span-01");
  testMalformedTraceparent("00-trace-invalid_span_id-01");
  
  testMalformedB3Headers("invalid", "span123");
  testMalformedB3Headers("trace123", "invalid");
  
  testMalformedBaggage("key=value;invalid_property");
  testMalformedBaggage("invalid%encoding");
}

// Example: Size limit edge cases
TEST_F(EdgeCaseTest, SizeLimitEdgeCases) {
  testBaggageAtSizeLimit(8192);
  testBaggageOverSizeLimit(8193);
  testLongHeaderValues();
  testManySmallBaggageEntries();
}
```

### Concurrent Access Testing

```cpp
// Example: Thread safety validation
TEST_F(ConcurrencyTest, ThreadSafetyValidation) {
  // Test concurrent extraction
  testConcurrentExtraction();
  
  // Test concurrent injection
  testConcurrentInjection();
  
  // Test concurrent baggage operations
  testConcurrentBaggageOperations();
}
```

## Test Utilities and Helpers

### Common Test Patterns

```cpp
// Reusable test helpers for consistent validation
class PropagatorTestBase {
protected:
  void validateTraceContext(const CompositeTraceContext& context, 
                           const std::string& expected_trace_id,
                           const std::string& expected_span_id,
                           bool expected_sampled);
  
  void validateBaggage(const CompositeBaggage& baggage,
                      const std::map<std::string, std::string>& expected_entries);
  
  void validateRoundTrip(const std::function<void()>& setup,
                        const std::function<void()>& extract_inject,
                        const std::function<void()>& verify);
};
```

This comprehensive testing strategy ensures that all propagator implementations maintain the highest standards for specification compliance, performance, and reliability across all supported distributed tracing formats.
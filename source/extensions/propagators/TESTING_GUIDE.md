# Propagator Testing Guide

This guide provides comprehensive testing strategies and examples for all propagator implementations in Envoy.

## Overview

Envoy's propagator implementations include extensive test suites to ensure specification compliance, performance, and interoperability. This guide covers testing strategies, execution commands, and best practices for validating propagator functionality.

## Test Organization

### Directory Structure

```
test/extensions/propagators/
├── w3c/
│   ├── tracecontext/
│   │   └── tracecontext_propagator_test.cc
│   ├── baggage/
│   │   └── baggage_propagator_test.cc
│   └── integration/
│       └── w3c_integration_test.cc
├── b3/
│   ├── multi/
│   │   └── b3_multi_propagator_test.cc
│   └── single/
│       └── b3_single_propagator_test.cc
├── xray/
│   └── xray_propagator_test.cc
├── skywalking/
│   └── skywalking_propagator_test.cc
└── integration/
    └── cross_propagator_test.cc
```

## Test Execution

### Run All Propagator Tests

```bash
# Run all propagator tests
bazel test //test/extensions/propagators/...

# Run with detailed output
bazel test //test/extensions/propagators/... --test_output=all

# Run with coverage
bazel coverage //test/extensions/propagators/...
```

### Run Specific Propagator Tests

```bash
# W3C propagators only
bazel test //test/extensions/propagators/w3c/...

# TraceContext propagator specifically
bazel test //test/extensions/propagators/w3c/tracecontext:tracecontext_propagator_test

# Baggage propagator specifically  
bazel test //test/extensions/propagators/w3c/baggage:baggage_propagator_test

# B3 propagators only
bazel test //test/extensions/propagators/b3/...

# Integration tests only
bazel test //test/extensions/propagators/integration/...
```

### Performance Testing

```bash
# Run performance benchmarks
bazel run //test/extensions/propagators/benchmarks:propagator_benchmark

# Memory usage tests
bazel test //test/extensions/propagators/... --test_arg=--memory_profile

# Load testing
bazel test //test/extensions/propagators/integration:load_test
```

## Test Categories

### 1. Unit Tests

Test individual propagator methods and functionality:

```cpp
// Example: TraceContext unit test structure
TEST(TraceContextPropagatorTest, ValidHeaderParsing) {
  TraceContextPropagator propagator;
  
  // Test valid traceparent parsing
  auto result = propagator.parseTraceParent("00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01");
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(result->version, "00");
  EXPECT_EQ(result->trace_id, "4bf92f3577b34da6a3ce929d0e0e4736");
  EXPECT_EQ(result->span_id, "00f067aa0ba902b7");
  EXPECT_TRUE(result->sampled);
}

TEST(TraceContextPropagatorTest, InvalidHeaderParsing) {
  TraceContextPropagator propagator;
  
  // Test various invalid formats
  auto result = propagator.parseTraceParent("invalid-header");
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), HasSubstr("Invalid traceparent format"));
}
```

### 2. Compliance Tests

Verify adherence to specifications:

```cpp
// Example: W3C specification compliance
class W3cComplianceTest : public testing::Test {
protected:
  void SetUp() override {
    propagator_ = std::make_unique<TraceContextPropagator>();
  }
  
  std::unique_ptr<TraceContextPropagator> propagator_;
};

TEST_F(W3cComplianceTest, TraceparentFieldSizes) {
  // Test W3C specification requirements
  
  // Version must be 2 hex characters
  auto result = propagator_->parseTraceParent("0-valid-trace-id-span-id-01");
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), HasSubstr("version must be 2 characters"));
  
  // Trace ID must be 32 hex characters
  result = propagator_->parseTraceParent("00-short-00f067aa0ba902b7-01");
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), HasSubstr("trace-id must be 32 characters"));
}

TEST_F(W3cComplianceTest, TraceparentCharacterValidation) {
  // All fields must be valid hex
  auto result = propagator_->parseTraceParent("00-4bf92f3577b34da6a3ce929d0e0e473g-00f067aa0ba902b7-01");
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), HasSubstr("Invalid hex character"));
}
```

### 3. Integration Tests

Test propagator usage within Envoy tracers:

```cpp
// Example: Tracer integration test
class TracerIntegrationTest : public testing::Test {
protected:
  void SetUp() override {
    // Set up test tracer with W3C propagator
    config_ = TestUtility::parseYaml<OpenTelemetryTracerConfig>(R"EOF(
      grpc_service:
        envoy_grpc:
          cluster_name: opentelemetry_collector
      service_name: envoy-test
    )EOF");
    
    tracer_ = std::make_unique<OpenTelemetryTracer>(config_, context_);
  }
  
  OpenTelemetryTracerConfig config_;
  std::unique_ptr<OpenTelemetryTracer> tracer_;
  Server::Configuration::TracerFactoryContext context_;
};

TEST_F(TracerIntegrationTest, W3cPropagationEndToEnd) {
  // Create span and test W3C header injection
  auto span = tracer_->startSpan("test-operation", {});
  
  Http::TestRequestHeaderMapImpl headers;
  Tracing::HttpTraceContext trace_context(headers);
  
  span->injectContext(trace_context, {});
  
  // Verify W3C headers are present and valid
  EXPECT_TRUE(headers.has("traceparent"));
  EXPECT_TRUE(headers.has("tracestate"));
  
  // Validate header format
  auto traceparent = headers.get_("traceparent");
  TraceContextPropagator propagator;
  auto parsed = propagator.parseTraceParent(traceparent);
  EXPECT_TRUE(parsed.ok());
}
```

### 4. Cross-Propagator Tests

Test interoperability between different propagation formats:

```cpp
// Example: B3 to W3C conversion test
TEST(CrossPropagatorTest, B3ToW3cConversion) {
  B3MultiPropagator b3_propagator;
  TraceContextPropagator w3c_propagator;
  
  // Set up B3 headers
  Tracing::TestTraceContextImpl context{
    {"X-B3-TraceId", "4bf92f3577b34da6a3ce929d0e0e4736"},
    {"X-B3-SpanId", "00f067aa0ba902b7"},
    {"X-B3-Sampled", "1"}
  };
  
  // Extract B3 context
  auto b3_trace_id = b3_propagator.extractTraceId(context);
  auto b3_span_id = b3_propagator.extractSpanId(context);
  auto b3_sampled = b3_propagator.extractSampled(context);
  
  // Convert to W3C format
  w3c_propagator.injectTraceParent(context, "00", *b3_trace_id, *b3_span_id, *b3_sampled);
  
  // Verify W3C format
  auto traceparent = w3c_propagator.extractTraceParent(context);
  EXPECT_TRUE(traceparent.has_value());
  
  auto parsed = w3c_propagator.parseTraceParent(*traceparent);
  EXPECT_TRUE(parsed.ok());
  EXPECT_EQ(parsed->trace_id, *b3_trace_id);
  EXPECT_EQ(parsed->span_id, *b3_span_id);
  EXPECT_EQ(parsed->sampled, *b3_sampled);
}
```

## Test Data and Fixtures

### Valid Test Cases

```cpp
// Common valid test data
constexpr char kValidTraceparent[] = "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01";
constexpr char kValidTracestate[] = "vendor1=value1,vendor2=value2";
constexpr char kValidBaggage[] = "userId=alice,sessionId=sess123,featureFlag=enabled";

// Edge case test data
constexpr char kMinimalTraceparent[] = "00-00000000000000000000000000000001-0000000000000001-00";
constexpr char kMaximalBaggage[] = "key=value"; // Construct max size baggage in test
```

### Invalid Test Cases

```cpp
// Common invalid test data for negative testing
const std::vector<std::string> kInvalidTraceparents = {
  "",                                                    // Empty
  "invalid",                                            // Wrong format
  "00-4bf92f3577b34da6a3ce929d0e0e473g-00f067aa0ba902b7-01", // Invalid hex
  "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7",     // Missing flags
  "00-00000000000000000000000000000000-00f067aa0ba902b7-01", // Zero trace ID
  "99-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01", // Unsupported version
};

const std::vector<std::string> kInvalidBaggage = {
  "=value",                    // Empty key
  "key=",                     // Empty value  
  "key;property",             // Missing value
  "key=value,",               // Trailing comma
  std::string(8193, 'x'),     // Exceeds size limit
};
```

## Performance Testing

### Benchmark Tests

```cpp
// Example: Performance benchmark
static void BM_TraceContextParsing(benchmark::State& state) {
  TraceContextPropagator propagator;
  constexpr char traceparent[] = "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01";
  
  for (auto _ : state) {
    auto result = propagator.parseTraceParent(traceparent);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_TraceContextParsing);

static void BM_BaggageExtraction(benchmark::State& state) {
  BaggagePropagator propagator;
  Tracing::TestTraceContextImpl context{
    {"baggage", "userId=alice,sessionId=sess123,feature=enabled"}
  };
  
  for (auto _ : state) {
    auto result = propagator.getBaggageValue(context, "userId");
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_BaggageExtraction);
```

### Memory Testing

```bash
# Run tests with memory sanitizer
bazel test //test/extensions/propagators/... --config=msan

# Check for memory leaks
bazel test //test/extensions/propagators/... --config=asan

# Profile memory usage
bazel run //test/extensions/propagators/tools:memory_profiler
```

## Load Testing

### High-Volume Testing

```cpp
// Example: Load test for propagator performance
TEST(LoadTest, HighVolumeTracePropagation) {
  TraceContextPropagator propagator;
  constexpr int kNumRequests = 100000;
  
  auto start = std::chrono::high_resolution_clock::now();
  
  for (int i = 0; i < kNumRequests; ++i) {
    std::string trace_id = generateRandomTraceId();
    std::string span_id = generateRandomSpanId();
    
    Tracing::TestTraceContextImpl context{};
    propagator.injectTraceParent(context, "00", trace_id, span_id, true);
    
    auto extracted = propagator.extractTraceParent(context);
    EXPECT_TRUE(extracted.has_value());
  }
  
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  
  // Verify performance requirements
  EXPECT_LT(duration.count(), 1000); // Should complete in < 1 second
}
```

## Continuous Integration

### GitHub Actions

```yaml
# Example CI configuration for propagator tests
name: Propagator Tests
on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    
    - name: Run Unit Tests
      run: bazel test //test/extensions/propagators/...
      
    - name: Run Integration Tests  
      run: bazel test //test/extensions/propagators/integration/...
      
    - name: Run Performance Tests
      run: bazel run //test/extensions/propagators/benchmarks:all
      
    - name: Generate Coverage Report
      run: bazel coverage //test/extensions/propagators/...
```

### Local Development

```bash
# Pre-commit testing script
#!/bin/bash
set -e

echo "Running propagator tests..."

# Unit tests
bazel test //test/extensions/propagators/w3c/...
bazel test //test/extensions/propagators/b3/...

# Integration tests
bazel test //test/extensions/propagators/integration/...

# Quick performance check
bazel run //test/extensions/propagators/benchmarks:quick_benchmark

echo "All tests passed!"
```

## Test Best Practices

### 1. Test Organization

- **Single Responsibility**: Each test should validate one specific behavior
- **Clear Naming**: Test names should describe what is being tested
- **Setup/Teardown**: Use proper test fixtures for consistent state

### 2. Data-Driven Testing

```cpp
// Example: Parameterized tests for multiple scenarios
class TraceparentValidationTest : public testing::TestWithParam<std::tuple<std::string, bool>> {};

TEST_P(TraceparentValidationTest, ValidateFormat) {
  auto [traceparent, should_be_valid] = GetParam();
  TraceContextPropagator propagator;
  
  auto result = propagator.parseTraceParent(traceparent);
  EXPECT_EQ(result.ok(), should_be_valid);
}

INSTANTIATE_TEST_SUITE_P(
  ValidAndInvalidCases,
  TraceparentValidationTest,
  testing::Values(
    std::make_tuple("00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01", true),
    std::make_tuple("invalid-format", false),
    std::make_tuple("", false)
  )
);
```

### 3. Error Testing

- **Negative Cases**: Test all error conditions
- **Edge Cases**: Test boundary conditions
- **Error Messages**: Verify error message quality

### 4. Performance Testing

- **Baseline Metrics**: Establish performance baselines
- **Regression Detection**: Monitor for performance regressions
- **Resource Usage**: Track memory and CPU usage

## Debugging Tests

### Common Issues

1. **Header Format Errors**: Check specification compliance
2. **Character Encoding**: Verify hex/URL encoding
3. **Size Limits**: Ensure compliance with size restrictions
4. **Memory Issues**: Use sanitizers to detect leaks

### Debug Commands

```bash
# Run single test with debugging
bazel test //test/extensions/propagators/w3c/tracecontext:tracecontext_propagator_test --test_filter="*ValidHeaderParsing*" --test_output=all

# Run with GDB
bazel run //test/extensions/propagators/w3c/tracecontext:tracecontext_propagator_test --run_under="gdb --args"

# Memory debugging
bazel test //test/extensions/propagators/... --config=asan --test_output=streamed
```

This testing guide ensures comprehensive validation of all propagator implementations, maintaining high quality and specification compliance.

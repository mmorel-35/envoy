# W3C Baggage Propagator

This implements the [W3C Baggage specification](https://www.w3.org/TR/baggage/) for correlation context propagation in distributed systems.

## Overview

The W3C Baggage specification defines a standard HTTP header for propagating correlation context (key-value pairs) across services:

- **`baggage`**: Contains key-value pairs in format `key1=value1,key2=value2;property`

Baggage allows you to attach metadata to traces that gets propagated to all downstream services, enabling correlation of requests across service boundaries.

## Features

- ✅ **Full W3C Compliance**: Implements the complete W3C Baggage specification
- ✅ **Size Limits**: Enforces 8KB total size and 180 member count limits
- ✅ **Validation**: Validates keys, values, and properties per specification
- ✅ **Properties Support**: Handles baggage member properties
- ✅ **Individual Access**: Get/set individual baggage values efficiently
- ✅ **URL Encoding**: Supports URL-encoded values
- ✅ **Error Handling**: Detailed validation and parsing error messages

## Usage

### Basic Usage

```cpp
#include "source/extensions/propagators/w3c/baggage/baggage_propagator.h"

BaggagePropagator propagator;

// Extract all baggage from incoming request
auto baggage_str = propagator.extractBaggage(trace_context);
if (baggage_str.has_value()) {
  auto baggage_map = propagator.parseBaggage(baggage_str.value());
  if (baggage_map.ok()) {
    // Access baggage members
    for (const auto& [key, member] : *baggage_map) {
      std::string key = member.key;
      std::string value = member.value;
      std::vector<std::string> properties = member.properties;
    }
  }
}
```

### Individual Baggage Operations

```cpp
// Get a specific baggage value
auto user_id = propagator.getBaggageValue(trace_context, "userId");
if (user_id.ok()) {
  std::string id = *user_id;  // "alice"
}

// Set individual baggage values
propagator.setBaggageValue(trace_context, "userId", "alice");
propagator.setBaggageValue(trace_context, "serverRegion", "us-west");
propagator.setBaggageValue(trace_context, "requestId", "req-12345");
```

### Advanced Usage with Baggage Maps

```cpp
// Create baggage map
BaggageMap baggage_map;

// Add members with properties
BaggageMember user_member;
user_member.key = "userId";
user_member.value = "alice";
user_member.properties = {"sensitive"};  // Optional properties

baggage_map["userId"] = user_member;

// Inject baggage map
propagator.injectBaggage(trace_context, baggage_map);

// Or inject from string directly
propagator.injectBaggage(trace_context, "userId=alice;sensitive,serverRegion=us-west");
```

### Utility Methods

```cpp
// Check if baggage is present
if (propagator.hasBaggage(trace_context)) {
  // Process existing baggage
}

// Remove all baggage
propagator.removeBaggage(trace_context);

// Serialize baggage map to string
std::string baggage_str = propagator.serializeBaggage(baggage_map);
```

## W3C Specification Compliance

### Baggage Header Format

The `baggage` header contains comma-separated key-value pairs:
```
key1=value1,key2=value2;property1;property2,key3=value3
```

**Examples:**
```
userId=alice,serverRegion=us-west
sessionId=abc123;secure,userId=bob,environment=production
requestType=api;version=v2;critical
```

### Size and Member Limits

✅ **Total Size Limit**: Maximum 8KB (8192 bytes) per specification
✅ **Member Count**: Practical limit of 180 members to prevent abuse
✅ **Key Length**: Maximum 256 characters per key
✅ **Value Length**: Maximum 4096 characters per value

### Validation Rules

**Key Validation:**
- Non-empty and max 256 characters
- Alphanumeric characters plus `_`, `-`, `.`, `*`
- Case-sensitive

**Value Validation:**
- Max 4096 characters
- URL-safe characters: alphanumeric plus `_`, `-`, `.`, `*`, `%`, `!`, `~`, `'`, `(`, `)`
- URL encoding supported

**Properties:**
- Optional semicolon-separated properties after value
- Same character validation as keys
- Used for metadata about baggage members

## Error Handling

The propagator provides specific error messages for validation failures:

```cpp
auto result = propagator.parseBaggage("invalid=");
if (!result.ok()) {
  // Detailed errors include:
  // - Size limit violations with actual vs max values
  // - Invalid character details for keys/values
  // - Member format errors with examples
  std::string error = result.status().message();
}
```

**Common Error Types:**
- `InvalidArgumentError`: Malformed baggage format
- `NotFoundError`: Requested key not found in baggage
- Size/count limit violations with specific details

## Use Cases

### Request Correlation
```cpp
// Service A: Set request tracking
propagator.setBaggageValue(trace_context, "requestId", "req-12345");
propagator.setBaggageValue(trace_context, "userId", "alice");

// Service B: Access correlation data
auto request_id = propagator.getBaggageValue(trace_context, "requestId");
auto user_id = propagator.getBaggageValue(trace_context, "userId");
```

### Feature Flags
```cpp
// Frontend: Set feature flags
propagator.setBaggageValue(trace_context, "featureFlag.newUI", "enabled");
propagator.setBaggageValue(trace_context, "featureFlag.betaAPI", "disabled");

// Backend: Check feature flags
auto ui_flag = propagator.getBaggageValue(trace_context, "featureFlag.newUI");
if (ui_flag.ok() && *ui_flag == "enabled") {
  // Use new UI logic
}
```

### Environment Context
```cpp
// Edge service: Set environment info
propagator.setBaggageValue(trace_context, "datacenter", "us-west-2");
propagator.setBaggageValue(trace_context, "environment", "production");
propagator.setBaggageValue(trace_context, "version", "v2.1.0");
```

## Performance Considerations

- **Lazy Parsing**: Baggage is only parsed when accessed
- **Size Validation**: Early size checks prevent resource exhaustion
- **Memory Efficient**: String views used during parsing to minimize allocations
- **Incremental Updates**: Individual value updates preserve existing baggage

## Integration

This propagator integrates with Envoy's tracing infrastructure:

1. **Header Constants**: Provides `W3cBaggageConstants` for consistent header access
2. **TraceContext API**: Uses standard `Tracing::TraceContext` interface  
3. **Status Handling**: Returns `absl::StatusOr` for robust error handling
4. **Streaming**: Supports both map-based and string-based operations

## Testing

Comprehensive test coverage includes:
- Valid/invalid baggage parsing
- Size and member limit enforcement
- Individual value get/set operations
- Properties handling
- Round-trip serialization
- UTF-8 and URL encoding scenarios
- Performance benchmarks

Run tests with:
```bash
bazel test //test/extensions/propagators/w3c/baggage:baggage_propagator_test
```

## Security Considerations

- **Size Limits**: Prevents unbounded memory usage
- **Character Validation**: Prevents injection attacks through header manipulation
- **No Sensitive Data**: Baggage is transmitted in HTTP headers - avoid secrets
- **Propagation Control**: Consider which baggage keys to forward downstream

## Standards References

- [W3C Baggage Specification](https://www.w3.org/TR/baggage/)
- [W3C Baggage GitHub Repository](https://github.com/w3c/baggage)
- [OpenTelemetry Baggage](https://opentelemetry.io/docs/specs/otel/baggage/api/)
- [HTTP State Management Mechanism](https://tools.ietf.org/html/rfc6265) (related concepts)
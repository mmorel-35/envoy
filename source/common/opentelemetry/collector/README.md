# OpenTelemetry Collector (DEPRECATED)

**⚠️ DEPRECATED: This directory has been superseded by `../proto/`. Please use the proto directory for OTLP protocol utilities.**

This directory previously contained shared OpenTelemetry OTLP utilities, but the functionality has been moved to better reflect its purpose.

## Migration

**Old (deprecated):**
```cpp
#include "source/common/opentelemetry/collector/otlp_utils.h"
using namespace Envoy::Common::OpenTelemetry::Collector;
```

**New (recommended):**
```cpp
#include "source/common/opentelemetry/proto/otlp_utils.h"
using namespace Envoy::Common::OpenTelemetry::Proto;
```

## Rationale

The functionality previously in this directory was more accurately described as OpenTelemetry protocol utilities rather than collector-specific logic. It includes helpers and constants that handle wire formats and protocol-level details relevant across multiple signals—not just collector functionality.

## See Also

- [Protocol Utilities](../proto/) - New location for OTLP protocol utilities
- [Traces](../traces/) - OpenTelemetry trace functionality  
- [Metrics](../metrics/) - OpenTelemetry metrics functionality
- [Logs](../logs/) - OpenTelemetry logs functionality

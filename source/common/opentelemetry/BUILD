load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

# Signal-specific libraries
envoy_cc_library(
    name = "traces_lib",
    deps = [
        "//source/common/opentelemetry/traces:traces_lib",
    ],
)

envoy_cc_library(
    name = "metrics_lib",
    deps = [
        "//source/common/opentelemetry/metrics:metrics_lib",
    ],
)

envoy_cc_library(
    name = "logs_lib",
    deps = [
        "//source/common/opentelemetry/logs:logs_lib",
    ],
)

envoy_cc_library(
    name = "proto_lib",
    deps = [
        "//source/common/opentelemetry/proto:proto_lib",
    ],
)

# Main library aggregating all OpenTelemetry functionality
envoy_cc_library(
    name = "opentelemetry_common_lib",
    deps = [
        ":proto_lib",
        ":logs_lib",
        ":metrics_lib",
        ":traces_lib",
    ],
)

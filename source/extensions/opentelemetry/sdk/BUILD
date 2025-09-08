load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "opentelemetry_sdk_lib",
    deps = [
        "//source/extensions/opentelemetry/sdk/common:sdk_common_lib",
        "//source/extensions/opentelemetry/sdk/trace:sdk_trace_lib",
        "//source/extensions/opentelemetry/sdk/metrics:sdk_metrics_lib", 
        "//source/extensions/opentelemetry/sdk/logs:sdk_logs_lib",
        # Additional signal-specific libraries will be added here as they are implemented
        # "//source/extensions/opentelemetry/sdk/resource:sdk_resource_lib",
        # "//source/extensions/opentelemetry/sdk/configuration:sdk_configuration_lib",
        # "//source/extensions/opentelemetry/sdk/version:sdk_version_lib",
    ],
)
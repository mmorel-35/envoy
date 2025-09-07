load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "otlp_utils_lib",
    deps = [
        "//source/common/opentelemetry/proto:otlp_utils_lib",
    ],
    deprecation = "Use //source/common/opentelemetry/proto:otlp_utils_lib instead",
)

envoy_cc_library(
    name = "collector_lib",
    deps = [
        ":otlp_utils_lib",
    ],
    deprecation = "Use //source/common/opentelemetry/proto:proto_lib instead",
)
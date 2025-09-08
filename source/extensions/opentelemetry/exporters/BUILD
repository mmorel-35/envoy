load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "exporters_lib",
    deps = [
        "//source/extensions/opentelemetry/exporters/otlp:otlp_utils_lib",
    ],
)
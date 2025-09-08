load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "sdk_lib",
    deps = [
        "//source/extensions/opentelemetry/sdk/common:types_lib",
        "//source/extensions/opentelemetry/sdk/version:version_lib",
    ],
)
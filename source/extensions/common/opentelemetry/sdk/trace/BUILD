load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "sdk_trace_types_lib",
    hdrs = ["types.h"],
    deps = [
        "@opentelemetry-proto//:trace_proto_cc",
        "@opentelemetry-proto//:trace_service_proto_cc",
    ],
)

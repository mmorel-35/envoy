load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "trace_context_lib",
    hdrs = ["trace_context.h"],
)

envoy_cc_library(
    name = "propagator_lib",
    hdrs = ["propagator.h"],
    deps = [
        ":trace_context_lib",
        "//source/common/singleton:const_singleton",
        "//source/common/tracing:trace_context_impl",
    ],
)
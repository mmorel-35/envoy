load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "constants_lib",
    hdrs = ["constants.h"],
)

envoy_cc_library(
    name = "propagator_lib",
    hdrs = ["propagator.h"],
    deps = [
        ":constants_lib",
        "//source/common/singleton:const_singleton",
        "//source/common/tracing:trace_context_impl",
    ],
)
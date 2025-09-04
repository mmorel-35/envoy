load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "propagator_constants_lib",
    hdrs = ["propagator_constants.h"],
    deps = [
        "//source/common/singleton:const_singleton",
        "//source/common/tracing:trace_context_impl",
    ],
)

envoy_cc_library(
    name = "propagator_interface_lib",
    hdrs = ["propagator_interface.h"],
    deps = [
        "//envoy/tracing:trace_context_interface",
        "//source/common/common:utility_lib",
    ],
)

envoy_cc_library(
    name = "all_propagators_lib",
    deps = [
        "//source/extensions/propagators/b3:b3_propagator_lib",
        "//source/extensions/propagators/w3c:w3c_propagator_lib", 
        "//source/extensions/propagators/xray:xray_propagator_lib",
    ],
)
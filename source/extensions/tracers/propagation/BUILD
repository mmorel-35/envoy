load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "all_propagators_constants_lib",
    deps = [
        "//source/extensions/tracers/propagation/b3:propagator_lib",
        "//source/extensions/tracers/propagation/w3c:propagator_lib", 
        "//source/extensions/tracers/propagation/xray:propagator_lib",
    ],
)
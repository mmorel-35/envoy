load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "w3c_propagator_lib",
    srcs = ["w3c_propagator.cc"],
    hdrs = ["w3c_propagator.h"],
    deps = [
        "//source/extensions/propagators:propagator_constants_lib",
        "//source/extensions/propagators:propagator_interface_lib",
        "//source/common/common:hex_lib",
        "//source/common/common:utility_lib",
        "@com_google_absl//absl/strings",
    ],
)
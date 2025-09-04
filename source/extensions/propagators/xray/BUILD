load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "xray_propagator_lib",
    srcs = ["xray_propagator.cc"],
    hdrs = ["xray_propagator.h"],
    deps = [
        "//source/extensions/propagators:propagator_constants_lib",
        "//source/extensions/propagators:propagator_interface_lib",
        "//source/common/common:hex_lib",
        "//source/common/common:utility_lib",
        "@com_google_absl//absl/strings",
    ],
)
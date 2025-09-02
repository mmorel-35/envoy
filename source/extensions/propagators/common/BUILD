load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "trace_id_utils_lib",
    srcs = ["trace_id_utils.cc"],
    hdrs = ["trace_id_utils.h"],
    deps = [
        "//envoy/common:time_interface",
        "@com_google_absl//absl/strings",
    ],
)
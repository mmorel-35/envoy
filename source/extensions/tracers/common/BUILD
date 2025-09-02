load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "factory_base_lib",
    hdrs = ["factory_base.h"],
    deps = [
        "//envoy/server:tracer_config_interface",
        "//source/common/config:utility_lib",
    ],
)

envoy_cc_library(
    name = "trace_id_utils_lib",
    srcs = ["trace_id_utils.cc"],
    hdrs = ["trace_id_utils.h"],
    deps = [
        "//envoy/common:time_interface",
        "@com_google_absl//absl/strings",
    ],
)

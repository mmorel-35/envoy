load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "version_lib",
    srcs = ["version.cc"],
    hdrs = ["version.h"],
    deps = [
        "//source/common/common:fmt_lib",
        "//source/common/common:macros",
        "//source/common/version:version_lib",
    ],
)
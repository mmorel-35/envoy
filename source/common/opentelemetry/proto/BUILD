load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "otlp_utils_lib",
    srcs = ["otlp_utils.cc"],
    hdrs = ["otlp_utils.h"],
    copts = [
        # Make sure that headers included from opentelemetry-api use Abseil from Envoy
        # https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.14.0/api/BUILD#L32
        "-DHAVE_ABSEIL",
    ],
    deps = [
        "//source/common/opentelemetry/traces:types_lib",
        "//source/common/common:fmt_lib",
        "//source/common/common:macros",
        "//source/common/version:version_lib",
        "@opentelemetry_proto//:common_proto_cc",
    ],
)

envoy_cc_library(
    name = "proto_lib",
    deps = [
        ":otlp_utils_lib",
    ],
)

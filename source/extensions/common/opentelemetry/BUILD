load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "otlp_types_lib",
    hdrs = ["types.h"],
    copts = [
        # Make sure that headers included from opentelemetry-api use Abseil from Envoy
        # https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.14.0/api/BUILD#L32
        "-DHAVE_ABSEIL",
    ],
    deps = [
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/types:variant",
        "@opentelemetry-proto//:common_proto_cc",
        "@opentelemetry-proto//:trace_proto_cc",
    ],
)

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
        ":otlp_types_lib",
        "//source/common/common:assert_lib",
        "//source/common/common:macros",
        "//source/common/version:version_lib",
    ],
)

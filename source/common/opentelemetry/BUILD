load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "protocol_constants_lib",
    hdrs = ["protocol_constants.h"],
    deps = [
        "@com_google_absl//absl/strings",
    ],
)

envoy_cc_library(
    name = "types_lib",
    hdrs = ["types.h"],
    deps = [
        "@io_opentelemetry_cpp//api",
        "@opentelemetry_proto//:common_proto_cc",
        "@opentelemetry_proto//:logs_service_proto_cc",
        "@opentelemetry_proto//:metrics_proto_cc",
        "@opentelemetry_proto//:metrics_service_proto_cc",
        "@opentelemetry_proto//:trace_proto_cc",
        "@opentelemetry_proto//:trace_service_proto_cc",
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
        ":types_lib",
        "//source/common/common:fmt_lib",
        "//source/common/common:macros",
        "//source/common/version:version_lib",
        "@opentelemetry_proto//:common_proto_cc",
    ],
)

envoy_cc_library(
    name = "opentelemetry_common_lib",
    deps = [
        ":otlp_utils_lib",
        ":protocol_constants_lib",
        ":types_lib",
    ],
)

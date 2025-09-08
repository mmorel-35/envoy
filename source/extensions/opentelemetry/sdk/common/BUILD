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
    name = "sdk_common_lib",
    deps = [
        ":protocol_constants_lib",
        ":types_lib",
    ],
)
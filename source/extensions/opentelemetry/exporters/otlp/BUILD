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
        "//source/extensions/opentelemetry/sdk/common:types_lib",
        "@com_google_absl//absl/strings",
        "@opentelemetry_proto//:common_proto_cc",
    ],
)

envoy_cc_library(
    name = "trace_exporter_lib",
    srcs = [
        "grpc_trace_exporter.cc", 
        "http_trace_exporter.cc",
    ],
    hdrs = [
        "grpc_trace_exporter.h",
        "http_trace_exporter.h",
        "trace_exporter.h",
    ],
    deps = [
        ":otlp_utils_lib",
        "//envoy/grpc:async_client_manager_interface",
        "//envoy/upstream:cluster_manager_interface",
        "//source/common/grpc:typed_async_client_lib",
        "//source/common/http:async_client_utility_lib",
        "//source/common/http:header_map_lib",
        "//source/common/http:message_lib",
        "//source/common/http:utility_lib",
        "//source/common/protobuf",
        "//source/common/tracing:trace_context_lib",
        "//source/extensions/opentelemetry/sdk/common:types_lib",
        "//source/extensions/opentelemetry/sdk/version:version_lib",
        "@envoy_api//envoy/config/core/v3:pkg_cc_proto",
        "@io_opentelemetry_cpp//api",
        "@opentelemetry_proto//:trace_proto_cc",
        "@opentelemetry_proto//:trace_service_proto_cc",
    ],
)
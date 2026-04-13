load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "environment_lib",
    srcs = ["environment.cc"],
    hdrs = ["environment.h"],
    deps = [
        "//source/common/common:macros",
        "//source/common/version:version_lib",
    ],
)

envoy_cc_library(
    name = "populate_attribute_utils_lib",
    srcs = ["populate_attribute_utils.cc"],
    hdrs = ["populate_attribute_utils.h"],
    copts = [
        # Make sure that headers included from opentelemetry-api use Abseil from Envoy
        # https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.14.0/api/BUILD#L32
        "-DHAVE_ABSEIL",
    ],
    deps = [
        "//source/common/common:assert_lib",
        "//source/extensions/common/opentelemetry:types_lib",
        "@opentelemetry-cpp//api",
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
        "//envoy/grpc:async_client_manager_interface",
        "//envoy/server:factory_context_interface",
        "//envoy/upstream:cluster_manager_interface",
        "//source/common/grpc:typed_async_client_lib",
        "//source/common/http:async_client_utility_lib",
        "//source/common/http:header_map_lib",
        "//source/common/http:http_service_headers_lib",
        "//source/common/http:message_lib",
        "//source/common/http:utility_lib",
        "//source/common/protobuf",
        "//source/common/tracing:trace_context_lib",
        "//source/common/version:version_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:environment_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:populate_attribute_utils_lib",
        "//source/extensions/common/opentelemetry/sdk/trace:constants_lib",
        "//source/server:generic_factory_context_lib",
        "@envoy_api//envoy/config/core/v3:pkg_cc_proto",
        "@opentelemetry-cpp//api",
        "@opentelemetry-proto//:trace_proto_cc",
        "@opentelemetry-proto//:trace_service_proto_cc",
    ],
)

envoy_cc_library(
    name = "metrics_exporter_lib",
    srcs = [
        "grpc_metrics_exporter.cc",
        "http_metrics_exporter.cc",
    ],
    hdrs = [
        "grpc_metrics_exporter.h",
        "http_metrics_exporter.h",
        "metrics_exporter.h",
    ],
    deps = [
        "//envoy/server:factory_context_interface",
        "//envoy/singleton:instance_interface",
        "//envoy/upstream:cluster_manager_interface",
        "//source/common/common:enum_to_int",
        "//source/common/grpc:status_lib",
        "//source/common/grpc:typed_async_client_lib",
        "//source/common/http:async_client_utility_lib",
        "//source/common/http:header_map_lib",
        "//source/common/http:http_service_headers_lib",
        "//source/common/http:message_lib",
        "//source/common/http:utility_lib",
        "//source/common/protobuf",
        "//source/common/tracing:null_span_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:environment_lib",
        "//source/extensions/common/opentelemetry/sdk/metrics:constants_lib",
        "@envoy_api//envoy/config/core/v3:pkg_cc_proto",
        "@opentelemetry-proto//:metrics_proto_cc",
        "@opentelemetry-proto//:metrics_service_proto_cc",
    ],
)

envoy_cc_library(
    name = "logs_utils_lib",
    srcs = ["logs_utils.cc"],
    hdrs = ["logs_utils.h"],
    deps = [
        "//envoy/formatter:http_formatter_context_interface",
        "//envoy/local_info:local_info_interface",
        "//envoy/stats:stats_interface",
        "//envoy/stream_info:filter_state_interface",
        "//envoy/stream_info:stream_info_interface",
        "//source/common/common:hex_lib",
        "//source/common/http:header_map_lib",
        "//source/common/protobuf:utility_lib",
        "//source/common/tracing:custom_tag_lib",
        "//source/common/tracing:http_tracer_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:environment_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:populate_attribute_utils_lib",
        "@abseil-cpp//absl/strings",
        "@envoy_api//envoy/data/accesslog/v3:pkg_cc_proto",
        "@envoy_api//envoy/extensions/access_loggers/open_telemetry/v3:pkg_cc_proto",
        "@opentelemetry-proto//:logs_proto_cc",
        "@opentelemetry-proto//:logs_service_proto_cc",
    ],
)

envoy_cc_library(
    name = "logs_exporter_lib",
    srcs = [
        "grpc_logs_exporter.cc",
        "http_logs_exporter.cc",
    ],
    hdrs = [
        "grpc_logs_exporter.h",
        "http_logs_exporter.h",
        "logs_exporter.h",
    ],
    deps = [
        "//envoy/event:dispatcher_interface",
        "//envoy/grpc:async_client_manager_interface",
        "//envoy/local_info:local_info_interface",
        "//envoy/server:factory_context_interface",
        "//envoy/singleton:instance_interface",
        "//envoy/thread_local:thread_local_interface",
        "//envoy/upstream:cluster_manager_interface",
        "//source/common/common:enum_to_int",
        "//source/common/common:logger_lib",
        "//source/common/config:utility_lib",
        "//source/common/grpc:typed_async_client_lib",
        "//source/common/http:async_client_utility_lib",
        "//source/common/http:header_map_lib",
        "//source/common/http:http_service_headers_lib",
        "//source/common/http:message_lib",
        "//source/common/http:utility_lib",
        "//source/common/protobuf",
        "//source/extensions/access_loggers/common:grpc_access_logger",
        "//source/extensions/access_loggers/common:grpc_access_logger_clients_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:logs_utils_lib",
        "//source/extensions/common/opentelemetry/sdk/logs:constants_lib",
        "@abseil-cpp//absl/container:flat_hash_map",
        "@envoy_api//envoy/extensions/access_loggers/grpc/v3:pkg_cc_proto",
        "@envoy_api//envoy/extensions/access_loggers/open_telemetry/v3:pkg_cc_proto",
        "@opentelemetry-proto//:logs_proto_cc",
        "@opentelemetry-proto//:logs_service_proto_cc",
    ],
)


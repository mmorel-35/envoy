load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_test(
    name = "populate_attribute_utils_test",
    srcs = ["populate_attribute_utils_test.cc"],
    copts = [
        # Make sure that headers included from opentelemetry-api use Abseil from Envoy
        # https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.14.0/api/BUILD#L32
        "-DHAVE_ABSEIL",
    ],
    rbe_pool = "6gig",
    deps = [
        "//source/common/version:version_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:environment_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:populate_attribute_utils_lib",
    ],
)

envoy_cc_test(
    name = "logs_utils_test",
    srcs = ["logs_utils_test.cc"],
    rbe_pool = "6gig",
    deps = [
        "//source/common/router:string_accessor_lib",
        "//source/common/stream_info:filter_state_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:logs_utils_lib",
        "//test/mocks/local_info:local_info_mocks",
        "//test/mocks/stream_info:stream_info_mocks",
        "//test/test_common:utility_lib",
        "@opentelemetry-proto//:logs_service_proto_cc",
        "@opentelemetry-proto//:resource_proto_cc",
    ],
)

envoy_cc_test(
    name = "metrics_exporter_test",
    srcs = ["metrics_exporter_test.cc"],
    rbe_pool = "6gig",
    deps = [
        "//source/common/tracing:null_span_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:metrics_exporter_lib",
        "//test/mocks/grpc:grpc_mocks",
        "//test/mocks/server:server_factory_context_mocks",
        "//test/mocks/upstream:upstream_mocks",
        "//test/test_common:utility_lib",
        "@opentelemetry-proto//:metrics_service_proto_cc",
    ],
)

envoy_cc_test(
    name = "logs_exporter_test",
    srcs = ["logs_exporter_test.cc"],
    rbe_pool = "6gig",
    deps = [
        "//source/common/buffer:zero_copy_input_stream_lib",
        "//source/common/grpc:common_lib",
        "//source/common/http:http_service_headers_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:logs_exporter_lib",
        "//test/mocks/event:event_mocks",
        "//test/mocks/grpc:grpc_mocks",
        "//test/mocks/local_info:local_info_mocks",
        "//test/mocks/server:factory_context_mocks",
        "//test/mocks/server:server_factory_context_mocks",
        "//test/mocks/stats:stats_mocks",
        "//test/mocks/thread_local:thread_local_mocks",
        "//test/mocks/upstream:upstream_mocks",
        "//test/test_common:utility_lib",
        "@envoy_api//envoy/config/core/v3:pkg_cc_proto",
        "@envoy_api//envoy/extensions/access_loggers/grpc/v3:pkg_cc_proto",
        "@opentelemetry-proto//:logs_proto_cc",
        "@opentelemetry-proto//:logs_service_proto_cc",
    ],
)


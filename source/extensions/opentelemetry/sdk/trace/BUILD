load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "span_context_lib",
    hdrs = ["span_context.h"],
    deps = [
        "//source/extensions/opentelemetry/sdk/common:types_lib",
    ],
)

envoy_cc_library(
    name = "span_context_extractor_lib", 
    srcs = ["span_context_extractor.cc"],
    hdrs = ["span_context_extractor.h"],
    deps = [
        ":span_context_lib",
        "//source/common/common:base_logger_lib",
        "//source/common/tracing:trace_context_lib",
    ],
)

envoy_cc_library(
    name = "tracer_lib",
    srcs = ["tracer.cc"],
    hdrs = ["tracer.h"],
    copts = [
        # Make sure that headers included from opentelemetry-api use Abseil from Envoy
        # https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.14.0/api/BUILD#L32
        "-DHAVE_ABSEIL",
    ],
    deps = [
        ":span_context_extractor_lib",
        ":span_context_lib",
        "//source/extensions/opentelemetry/exporters/otlp:trace_exporter_lib",
        "//source/extensions/opentelemetry/sdk/resource:resource_detector_lib",
        "//source/extensions/opentelemetry/sdk/trace/samplers:sampler_lib",
        "//source/extensions/tracers/common:factory_base_lib",
        "@envoy_api//envoy/config/trace/v3:pkg_cc_proto",
        "@io_opentelemetry_cpp//api",
        "@opentelemetry_proto//:trace_proto_cc",
        "@opentelemetry_proto//:trace_service_proto_cc",
    ],
)

envoy_cc_library(
    name = "opentelemetry_tracer_impl_lib",
    srcs = ["opentelemetry_tracer_impl.cc"],
    hdrs = ["opentelemetry_tracer_impl.h"],
    deps = [
        ":tracer_lib",
        "//source/extensions/opentelemetry/sdk/resource:resource_detector_lib",
        "//source/extensions/opentelemetry/sdk/trace/samplers:sampler_lib",
        "//source/extensions/tracers/common:factory_base_lib",
        "@envoy_api//envoy/config/trace/v3:pkg_cc_proto",
    ],
)
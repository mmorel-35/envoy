load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "propagator_interface_lib",
    srcs = [
        "propagator.cc",
    ],
    hdrs = [
        "propagator.h",
    ],
    deps = [
        "//source/common/common:logger_lib",
        "//source/common/common:statusor_lib",
        "//source/common/http:header_map_lib",
        "//source/common/tracing:trace_context_lib",
        "//source/extensions/tracers/opentelemetry:span_context_lib",
        "@io_opentelemetry_cpp//api",
    ],
)

envoy_cc_library(
    name = "trace_context_lib",
    srcs = [
        "trace_context.cc",
    ],
    hdrs = [
        "trace_context.h",
    ],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

envoy_cc_library(
    name = "type_converter_lib",
    srcs = [
        "type_converter.cc",
    ],
    hdrs = [
        "type_converter.h",
    ],
    deps = [
        ":trace_context_lib",
        "//source/extensions/tracers/opentelemetry:span_context_lib",
        "@com_google_absl//absl/strings",
    ],
)

envoy_cc_library(
    name = "generic_propagator_interface_lib",
    srcs = [
        "generic_propagator.cc",
    ],
    hdrs = [
        "generic_propagator.h",
    ],
    deps = [
        ":trace_context_lib",
        "//source/common/common:logger_lib",
        "//source/common/common:statusor_lib",
        "//source/common/tracing:trace_context_lib",
    ],
)

envoy_cc_library(
    name = "propagator_lib",
    srcs = [
        "propagator_factory.cc",
    ],
    hdrs = [
        "propagator_factory.h",
    ],
    deps = [
        ":propagator_interface_lib",
        ":generic_propagator_interface_lib", 
        ":type_converter_lib",
        "//envoy/api:api_interface",
        "//source/common/common:utility_lib",
        "//source/common/config:datasource_lib",
        "//source/extensions/propagators/b3:b3_propagator_lib",
        "//source/extensions/propagators/b3:generic_b3_propagator_lib",
        "//source/extensions/propagators/w3c:w3c_propagators_lib",
        "//source/extensions/propagators/w3c:generic_w3c_propagators_lib",
        "@envoy_api//envoy/config/core/v3:pkg_cc_proto",
        "@envoy_api//envoy/config/trace/v3:pkg_cc_proto",
    ],
)
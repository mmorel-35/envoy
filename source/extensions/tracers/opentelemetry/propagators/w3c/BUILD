load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "w3c_propagators_lib",
    srcs = [
        "w3c_trace_context_propagator.cc",
        "baggage_propagator.cc",
    ],
    hdrs = [
        "w3c_trace_context_propagator.h",
        "baggage_propagator.h",
    ],
    deps = [
        "//source/extensions/tracers/opentelemetry/propagators:propagator_lib",
        "//source/common/http:header_map_lib",
        "//source/common/tracing:trace_context_lib",
        "@io_opentelemetry_cpp//api",
    ],
)
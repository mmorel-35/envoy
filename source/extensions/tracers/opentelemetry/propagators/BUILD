load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "propagator_lib",
    srcs = [
        "propagator.cc",
        "propagator_factory.cc",
    ],
    hdrs = [
        "propagator.h",
        "propagator_factory.h",
    ],
    deps = [
        "//source/common/http:header_map_lib",
        "//source/common/tracing:trace_context_lib",
        "@envoy_api//envoy/config/trace/v3:pkg_cc_proto",
        "@io_opentelemetry_cpp//api",
    ],
)
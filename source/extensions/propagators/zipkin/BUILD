load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "propagator_factory_lib",
    srcs = ["propagator_factory.cc"],
    hdrs = ["propagator_factory.h"],
    deps = [
        "//envoy/api:api_interface",
        "//source/common/common:logger_lib",
        "//source/extensions/propagators/opentelemetry:propagator_lib",
        "//source/extensions/propagators/opentelemetry/b3:b3_propagator_lib",
        "//source/extensions/propagators/opentelemetry/w3c:w3c_trace_context_propagator_lib",
    ],
)
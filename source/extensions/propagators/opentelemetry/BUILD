load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "trace_context_lib",
    srcs = ["trace_context.cc"],
    hdrs = ["trace_context.h"],
    deps = [
        "//source/extensions/propagators/w3c:trace_context_lib",
        "//source/extensions/propagators/b3:trace_context_lib",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:variant",
        "@com_google_absl//absl/strings:str_format",
    ],
)

envoy_cc_library(
    name = "propagator_lib",
    srcs = ["propagator.cc"],
    hdrs = ["propagator.h"],
    deps = [
        ":trace_context_lib",
        "//source/extensions/propagators/w3c:propagator_lib",
        "//source/extensions/propagators/b3:propagator_lib",
        "//envoy/tracing:trace_context_interface",
        "//source/common/tracing:trace_context_lib",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/strings:str_format",
    ],
)

envoy_cc_library(
    name = "integration_example_lib",
    hdrs = ["integration_example.h"],
    deps = [
        ":propagator_lib",
        "//envoy/tracing:tracer_interface",
    ],
)
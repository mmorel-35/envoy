load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library")

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

envoy_cc_library(
    name = "b3_trace_context_lib",
    srcs = ["trace_context.cc"],
    hdrs = ["trace_context.h"],
    deps = [
        "//source/common/common:utility_lib",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

envoy_cc_library(
    name = "b3_propagator_lib", 
    srcs = ["propagator.cc"],
    hdrs = ["propagator.h"],
    deps = [
        ":b3_trace_context_lib",
        "//envoy/tracing:tracer_interface",
        "//source/common/common:utility_lib",
        "//source/common/tracing:trace_context_lib",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor", 
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)
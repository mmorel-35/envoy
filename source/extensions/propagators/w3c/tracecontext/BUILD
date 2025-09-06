load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "tracecontext_propagator_lib",
    srcs = ["tracecontext_propagator.cc"],
    hdrs = ["tracecontext_propagator.h"],
    deps = [
        "//source/common/common:hex_lib",
        "//source/common/singleton:const_singleton",
        "//source/common/tracing:trace_context_lib",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

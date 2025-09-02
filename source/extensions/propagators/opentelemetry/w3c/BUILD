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
        "//source/extensions/propagators:propagator_interface_lib",
        "//source/common/common:hex_lib",
        "//source/common/tracing:trace_context_lib",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
    ],
)
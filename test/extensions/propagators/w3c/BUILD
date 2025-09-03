load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_test(
    name = "trace_context_test",
    srcs = ["trace_context_test.cc"],
    deps = [
        "//source/extensions/propagators/w3c:trace_context_lib",
        "//test/test_common:utility_lib",
    ],
)

envoy_cc_test(
    name = "propagator_test",
    srcs = ["propagator_test.cc"],
    deps = [
        "//source/extensions/propagators/w3c:propagator_lib",
        "//source/common/tracing:trace_context_impl",
        "//test/test_common:utility_lib",
    ],
)
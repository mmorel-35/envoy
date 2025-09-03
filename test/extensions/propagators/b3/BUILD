load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_cc_test_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_test(
    name = "trace_context_test",
    srcs = ["trace_context_test.cc"],
    deps = [
        "//source/extensions/propagators/b3:trace_context_lib",
        "//test/test_common:utility_lib",
        "@com_google_googletest//:gtest",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
    ],
)

envoy_cc_test(
    name = "propagator_test",
    srcs = ["propagator_test.cc"],
    deps = [
        "//source/extensions/propagators/b3:propagator_lib",
        "//source/common/http:header_map_lib",
        "//source/common/tracing:trace_context_lib",
        "//test/test_common:utility_lib",
        "@com_google_googletest//:gtest",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
    ],
)
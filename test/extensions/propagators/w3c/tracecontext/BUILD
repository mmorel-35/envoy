load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_test(
    name = "tracecontext_propagator_test",
    srcs = ["tracecontext_propagator_test.cc"],
    deps = [
        "//source/extensions/propagators/w3c/tracecontext:tracecontext_propagator_lib",
        "//test/test_common:utility_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
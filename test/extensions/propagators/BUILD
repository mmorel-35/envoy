load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_test(
    name = "propagator_constants_test",
    srcs = ["propagator_constants_test.cc"],
    deps = [
        "//source/extensions/propagators:propagator_constants_lib",
        "//test/test_common:utility_lib",
    ],
)
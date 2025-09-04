load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_test(
    name = "w3c_propagator_integration_test",
    srcs = ["w3c_propagator_integration_test.cc"],
    deps = [
        "//source/extensions/propagators/w3c:propagator_lib",
        "//source/extensions/tracers/opentelemetry:opentelemetry_tracer_lib",
        "//source/extensions/tracers/zipkin:zipkin_lib",
        "//test/test_common:utility_lib",
        "@com_google_googletest//:gtest",
    ],
)
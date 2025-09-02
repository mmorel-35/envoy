load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "opentelemetry_propagators_lib",
    deps = [
        "//source/extensions/propagators/opentelemetry/b3:b3_propagator_lib",
        "//source/extensions/propagators/opentelemetry/w3c:w3c_propagators_lib",
    ],
)
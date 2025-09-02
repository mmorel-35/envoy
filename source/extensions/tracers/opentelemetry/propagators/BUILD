load(
    "//bazel:envoy_build_system.bzl",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

# Re-export the propagator library from its actual location
# This maintains backward compatibility for tests that expect the library here
alias(
    name = "propagator_lib",
    actual = "//source/extensions/propagators/opentelemetry:propagator_lib",
    visibility = ["//visibility:public"],
)
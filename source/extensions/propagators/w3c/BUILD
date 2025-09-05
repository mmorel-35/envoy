load(
    "//bazel:envoy_build_system.bzl",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

# Alias libraries for backward compatibility
alias(
    name = "trace_context_lib",
    actual = "//source/extensions/propagators/w3c/tracecontext:trace_context_lib",
    visibility = ["//visibility:public"],
)

alias(
    name = "baggage_lib",
    actual = "//source/extensions/propagators/w3c/baggage:baggage_lib", 
    visibility = ["//visibility:public"],
)

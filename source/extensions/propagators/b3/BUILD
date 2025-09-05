load(
    "//bazel:envoy_build_system.bzl",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

# Alias libraries for backward compatibility
alias(
    name = "multi_lib",
    actual = "//source/extensions/propagators/b3/multi:multi_lib",
    visibility = ["//visibility:public"],
)

alias(
    name = "single_lib", 
    actual = "//source/extensions/propagators/b3/single:single_lib",
    visibility = ["//visibility:public"],
)

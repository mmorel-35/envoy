load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "all_propagators_constants_lib",
    deps = [
        "//source/extensions/propagators/b3:constants_lib",
        "//source/extensions/propagators/w3c:constants_lib", 
        "//source/extensions/propagators/xray:constants_lib",
    ],
)
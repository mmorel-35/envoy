load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "otlp_types_lib",
    hdrs = ["types.h"],
    copts = [
        # Make sure that headers included from opentelemetry-api use Abseil from Envoy
        # https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.14.0/api/BUILD#L32
        "-DHAVE_ABSEIL",
    ],
    deps = [
        "//source/extensions/common/opentelemetry/sdk/common:sdk_common_types_lib",
        "@opentelemetry-proto//:common_proto_cc",
        "@opentelemetry-proto//:trace_proto_cc",
    ],
)

envoy_cc_library(
    name = "populate_attribute_utils_lib",
    srcs = ["populate_attribute_utils.cc"],
    hdrs = ["populate_attribute_utils.h"],
    copts = [
        # Make sure that headers included from opentelemetry-api use Abseil from Envoy
        # https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.14.0/api/BUILD#L32
        "-DHAVE_ABSEIL",
    ],
    deps = [
        ":otlp_types_lib",
        "//source/common/common:assert_lib",
    ],
)

# Forwarding target for backward compatibility.
# New code should depend on :populate_attribute_utils_lib and/or
# //source/extensions/common/opentelemetry/exporters/otlp:environment_lib directly.
envoy_cc_library(
    name = "otlp_utils_lib",
    hdrs = ["otlp_utils.h"],
    copts = [
        # Make sure that headers included from opentelemetry-api use Abseil from Envoy
        # https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.14.0/api/BUILD#L32
        "-DHAVE_ABSEIL",
    ],
    deps = [
        ":populate_attribute_utils_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:environment_lib",
    ],
)

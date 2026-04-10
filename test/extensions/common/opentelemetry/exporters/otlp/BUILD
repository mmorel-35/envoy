load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_test(
    name = "populate_attribute_utils_test",
    srcs = ["populate_attribute_utils_test.cc"],
    copts = [
        # Make sure that headers included from opentelemetry-api use Abseil from Envoy
        # https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.14.0/api/BUILD#L32
        "-DHAVE_ABSEIL",
    ],
    deps = [
        "//source/extensions/common/opentelemetry/exporters/otlp:environment_lib",
        "//source/extensions/common/opentelemetry/exporters/otlp:populate_attribute_utils_lib",
        "//test/test_common:utility_lib",
        "@opentelemetry-proto//:common_proto_cc",
    ],
)

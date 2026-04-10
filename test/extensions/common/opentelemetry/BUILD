load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_test(
    name = "otlp_utils_test",
    srcs = ["otlp_utils_test.cc"],
    copts = [
        # Make sure that headers included from opentelemetry-api use Abseil from Envoy
        # https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.14.0/api/BUILD#L32
        "-DHAVE_ABSEIL",
    ],
    deps = [
        "//source/extensions/common/opentelemetry:otlp_utils_lib",
        "//test/test_common:utility_lib",
        "@opentelemetry-proto//:common_proto_cc",
    ],
)

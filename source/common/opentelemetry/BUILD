load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

licenses(["notice"])  # Apache 2

envoy_package()

# Signal-specific libraries
envoy_cc_library(
    name = "traces_lib",
    deps = [
        "//source/common/opentelemetry/traces:traces_lib",
    ],
)

envoy_cc_library(
    name = "metrics_lib",
    deps = [
        "//source/common/opentelemetry/metrics:metrics_lib",
    ],
)

envoy_cc_library(
    name = "logs_lib",
    deps = [
        "//source/common/opentelemetry/logs:logs_lib",
    ],
)

envoy_cc_library(
    name = "collector_lib",
    deps = [
        "//source/common/opentelemetry/collector:collector_lib",
    ],
)

# Backward compatibility libraries - DEPRECATED
# These are maintained for backward compatibility and will be removed in a future release.
# Please use the signal-specific libraries instead.
envoy_cc_library(
    name = "protocol_constants_lib",
    hdrs = ["protocol_constants.h"],
    deps = [
        "@com_google_absl//absl/strings",
    ],
    deprecation = "Use signal-specific constants from //source/common/opentelemetry/{traces,metrics,logs}:constants_lib instead",
)

envoy_cc_library(
    name = "types_lib",
    hdrs = ["types.h"],
    deps = [
        "@io_opentelemetry_cpp//api",
        "@opentelemetry_proto//:common_proto_cc",
        "@opentelemetry_proto//:logs_service_proto_cc",
        "@opentelemetry_proto//:metrics_proto_cc",
        "@opentelemetry_proto//:metrics_service_proto_cc",
        "@opentelemetry_proto//:trace_proto_cc",
        "@opentelemetry_proto//:trace_service_proto_cc",
    ],
    deprecation = "Use signal-specific types from //source/common/opentelemetry/{traces,metrics,logs}:types_lib instead",
)

envoy_cc_library(
    name = "otlp_utils_lib",
    srcs = ["otlp_utils.cc"],
    hdrs = ["otlp_utils.h"],
    copts = [
        # Make sure that headers included from opentelemetry-api use Abseil from Envoy
        # https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.14.0/api/BUILD#L32
        "-DHAVE_ABSEIL",
    ],
    deps = [
        ":types_lib",
        "//source/common/common:fmt_lib",
        "//source/common/common:macros",
        "//source/common/version:version_lib",
        "@opentelemetry_proto//:common_proto_cc",
    ],
    deprecation = "Use //source/common/opentelemetry/collector:otlp_utils_lib instead",
)

# Main library aggregating all OpenTelemetry functionality
envoy_cc_library(
    name = "opentelemetry_common_lib",
    deps = [
        ":collector_lib",
        ":logs_lib",
        ":metrics_lib",
        ":traces_lib",
        # Backward compatibility - these will be removed in a future release
        ":otlp_utils_lib",
        ":protocol_constants_lib",
        ":types_lib",
    ],
)

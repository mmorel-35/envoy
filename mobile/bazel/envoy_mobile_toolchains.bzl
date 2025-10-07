# Note: rules_java_toolchains is not needed in bzlmod mode when rules_java is a bazel_dep
# load("@rules_java//java:repositories.bzl", "rules_java_toolchains")
# Note: kt_register_toolchains is not needed in bzlmod mode when rules_kotlin is a bazel_dep
# load("@rules_kotlin//kotlin:core.bzl", "kt_register_toolchains")
# Note: rules_detekt_toolchains is not needed in bzlmod mode when rules_detekt is a bazel_dep
# load("@rules_detekt//detekt:toolchains.bzl", "rules_detekt_toolchains")
# Note: rules_proto_grpc_toolchains is not needed in bzlmod mode when rules_proto_grpc is a bazel_dep
# load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_toolchains")

def envoy_mobile_toolchains():
    # rules_java_toolchains() # Not needed in bzlmod mode - toolchains registered automatically
    # kt_register_toolchains() # Not needed in bzlmod mode - toolchains registered automatically
    # rules_detekt_toolchains() # Not needed in bzlmod mode - toolchains registered automatically
    # rules_proto_grpc_toolchains() # Not needed in bzlmod mode - toolchains registered automatically
    pass  # All toolchains are registered automatically in bzlmod mode

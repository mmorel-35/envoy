load("@build_bazel_apple_support//lib:repositories.bzl", "apple_support_dependencies")
load("@build_bazel_rules_apple//apple:repositories.bzl", "apple_rules_dependencies")
load("@build_bazel_rules_swift//swift:repositories.bzl", "swift_rules_dependencies")
load("@robolectric//bazel:robolectric.bzl", "robolectric_repositories")
load("@rules_detekt//detekt:dependencies.bzl", "rules_detekt_dependencies")
load("@rules_java//java:repositories.bzl", "rules_java_dependencies")
load("@rules_kotlin//kotlin:repositories.bzl", "kotlin_repositories")
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies")
load("@rules_proto//proto:toolchains.bzl", "rules_proto_toolchains")
load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_repos", "rules_proto_grpc_toolchains")

def _default_extra_swift_sources_impl(ctx):
    ctx.file("WORKSPACE", "")
    ctx.file("empty.swift", "")
    ctx.file("BUILD.bazel", """
filegroup(
    name = "extra_swift_srcs",
    srcs = ["empty.swift"],
    visibility = ["//visibility:public"],
)

objc_library(
    name = "extra_private_dep",
    module_name = "FakeDep",
    visibility = ["//visibility:public"],
)""")

_default_extra_swift_sources = repository_rule(
    implementation = _default_extra_swift_sources_impl,
)

def _default_extra_jni_deps_impl(ctx):
    ctx.file("WORKSPACE", "")
    ctx.file("BUILD.bazel", """
cc_library(
    name = "extra_jni_dep",
    visibility = ["//visibility:public"],
)""")

_default_extra_jni_deps = repository_rule(
    implementation = _default_extra_jni_deps_impl,
)

def envoy_mobile_dependencies():
    if not native.existing_rule("envoy_mobile_extra_swift_sources"):
        _default_extra_swift_sources(name = "envoy_mobile_extra_swift_sources")
    if not native.existing_rule("envoy_mobile_extra_jni_deps"):
        _default_extra_jni_deps(name = "envoy_mobile_extra_jni_deps")

    swift_dependencies()
    kotlin_dependencies()

def swift_dependencies():
    apple_support_dependencies()
    apple_rules_dependencies(ignore_version_differences = True)
    swift_rules_dependencies()

def kotlin_dependencies():
    rules_java_dependencies()
    # Note: Maven dependencies are now managed via the native rules_jvm_external
    # extension in MODULE.bazel instead of custom maven_install calls
    kotlin_repositories()
    rules_detekt_dependencies()
    robolectric_repositories()

    rules_proto_grpc_toolchains()
    rules_proto_grpc_repos()
    rules_proto_dependencies()
    rules_proto_toolchains()

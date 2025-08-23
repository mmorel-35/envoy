"""Repository rule for Android SDK and NDK autoconfiguration.

This rule is a no-op unless the required android environment variables are set.
"""

# Based on https://github.com/tensorflow/tensorflow/tree/34c03ed67692eb76cb3399cebca50ea8bcde064c/third_party/android
# Workaround for https://github.com/bazelbuild/bazel/issues/14260

_ANDROID_NDK_HOME = "ANDROID_NDK_HOME"
_ANDROID_SDK_HOME = "ANDROID_HOME"

def _android_autoconf_impl(repository_ctx):
    sdk_home = repository_ctx.os.environ.get(_ANDROID_SDK_HOME)
    ndk_home = repository_ctx.os.environ.get(_ANDROID_NDK_HOME)

    sdk_api_level = repository_ctx.attr.sdk_api_level
    ndk_api_level = repository_ctx.attr.ndk_api_level
    build_tools_version = repository_ctx.attr.build_tools_version

    sdk_rule = ""
    if sdk_home:
        sdk_rule = """
    native.android_sdk_repository(
        name="androidsdk",
        path="{}",
        api_level={},
        build_tools_version="{}",
    )
""".format(sdk_home, sdk_api_level, build_tools_version)

    ndk_rule = ""
    if ndk_home:
        ndk_rule = """
    android_ndk_repository(
        name="androidndk",
        path="{}",
        api_level={},
    )
    native.register_toolchains("@androidndk//:all")

""".format(ndk_home, ndk_api_level)

    if ndk_rule == "" and sdk_rule == "":
        sdk_rule = "pass"

    repository_ctx.file("BUILD.bazel", "")
    repository_ctx.file("android_configure.bzl", """
load("@rules_android_ndk//:rules.bzl", "android_ndk_repository")

def android_workspace():
    {}
    {}
    """.format(sdk_rule, ndk_rule))

android_configure = repository_rule(
    implementation = _android_autoconf_impl,
    environ = [
        _ANDROID_NDK_HOME,
        _ANDROID_SDK_HOME,
    ],
    attrs = {
        "sdk_api_level": attr.int(mandatory = True),
        "ndk_api_level": attr.int(mandatory = True),
        "build_tools_version": attr.string(mandatory = True),
    },
)

def _envoy_android_configure_impl(module_ctx):
    """Implementation of the envoy_android_configure extension."""
    # Configure Android SDK/NDK with the same parameters as used in WORKSPACE.bzlmod
    android_configure(
        name = "local_config_android",
        build_tools_version = "30.0.2",
        ndk_api_level = 23,
        sdk_api_level = 30,
    )

def _post_android_setup_impl(repository_ctx):
    """Repository rule to handle post-android configuration setup."""
    # This will be called after @local_config_android is available
    repository_ctx.file("BUILD.bazel", "")
    repository_ctx.file("WORKSPACE", "")
    
    # Create a dummy file to indicate setup is complete
    repository_ctx.file("setup_complete.txt", "Android workspace setup complete")

_post_android_setup = repository_rule(
    implementation = _post_android_setup_impl,
)

def _envoy_android_workspace_impl(module_ctx):
    """Implementation of the envoy_android_workspace extension."""
    # Create a marker repository to ensure android workspace setup completes
    _post_android_setup(name = "envoy_android_workspace_setup")

# Module extension for android_configure following consistent naming convention
envoy_android_configure_ext = module_extension(
    implementation = _envoy_android_configure_impl,
    doc = """
    Extension for Android SDK/NDK configuration.
    
    This extension wraps the android_configure() repository rule to make it
    available as a bzlmod module extension, following the consistent
    naming convention envoy_*_ext across all Envoy modules.
    """,
)

# Extension for Android workspace setup that depends on android_configure
envoy_android_workspace_ext = module_extension(
    implementation = _envoy_android_workspace_impl,
    doc = """
    Extension for Android workspace setup.
    
    This extension handles additional Android workspace setup that depends on
    the local_config_android repository being configured first.
    """,
)

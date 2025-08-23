"""Extension for Android configuration."""

load("//bazel:android_configure.bzl", "android_configure")

def _envoy_android_impl(module_ctx):
    """Implementation for envoy_android extension.
    
    This extension wraps the android_configure() repository rule to make it
    available as a bzlmod module extension.
    """
    # Configure Android SDK/NDK with the same parameters as used in WORKSPACE.bzlmod
    android_configure(
        name = "local_config_android",
        build_tools_version = "30.0.2",
        ndk_api_level = 23,
        sdk_api_level = 30,
    )

# Module extension for android_configure
android = module_extension(
    implementation = _envoy_android_impl,
    doc = """
    Extension for Android configuration.
    
    This extension wraps the android_configure() repository rule to make it
    available as a bzlmod module extension, handling Android SDK/NDK
    setup and configuration.
    """,
)
"""Extension for Envoy Mobile toolchains."""

load("//bazel:envoy_mobile_toolchains.bzl", "envoy_mobile_toolchains")

def _envoy_mobile_toolchains_impl(module_ctx):
    """Implementation for envoy_mobile_toolchains extension.
    
    This extension wraps the envoy_mobile_toolchains() function to make it
    available as a bzlmod module extension.
    """
    # Call the mobile toolchains function
    envoy_mobile_toolchains()

# Module extension for envoy_mobile_toolchains
envoy_mobile_toolchains = module_extension(
    implementation = _envoy_mobile_toolchains_impl,
    doc = """
    Extension for Envoy Mobile toolchains.
    
    This extension wraps the envoy_mobile_toolchains() function to make it
    available as a bzlmod module extension, handling mobile toolchain
    registration and configuration.
    """,
)
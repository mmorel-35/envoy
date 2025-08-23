"""Public extension for Envoy Mobile dependencies."""

load("//mobile/bazel:envoy_mobile_dependencies.bzl", "envoy_mobile_dependencies")

def _envoy_mobile_impl(module_ctx):
    """Implementation for envoy_mobile extension.
    
    This extension wraps the envoy_mobile_dependencies() function to make it
    available as a bzlmod module extension.
    """
    # Call the mobile dependencies function
    envoy_mobile_dependencies()

# Module extension for envoy_mobile_dependencies
envoy_mobile = module_extension(
    implementation = _envoy_mobile_impl,
    doc = """
    Extension for Envoy Mobile dependencies.
    
    This extension wraps the envoy_mobile_dependencies() function to make it
    available as a bzlmod module extension, handling mobile-specific
    dependencies including Swift, Kotlin, and Android tooling.
    """,
)
"""Extension for Envoy's main dependencies."""

load("//bazel:repositories.bzl", "envoy_dependencies")

# Bzlmod context detection - in bzlmod, labels start with @@
_IS_BZLMOD = str(Label("//:invalid")).startswith("@@")

def _dependencies_impl(module_ctx):
    """Implementation for dependencies extension.
    
    This extension wraps the envoy_dependencies() function to make it
    available as a bzlmod module extension.
    """
    # Call the main dependencies function
    envoy_dependencies()

# Module extension for envoy_dependencies  
dependencies = module_extension(
    implementation = _dependencies_impl,
    doc = """
    Extension for Envoy's main dependencies.
    
    This extension wraps the envoy_dependencies() function to make it
    available as a bzlmod module extension, following the streamlined
    bzlmod extension organization.
    """,
)
"""Extension for Envoy's main dependencies.

DEPRECATED: This extension has been consolidated into core.bzl for better maintainability.
Please use //bazel/extensions:core.bzl instead.

See BZLMOD_RECOMMENDATIONS.md for details on the consolidation effort.
"""

load("//bazel:repositories.bzl", "envoy_dependencies")

def _dependencies_impl(module_ctx):
    """Implementation for dependencies extension.

    DEPRECATED: Use //bazel/extensions:core.bzl instead.
    
    This extension wraps the envoy_dependencies() function to make it
    available as a bzlmod module extension.
    """

    # Call the main dependencies function
    envoy_dependencies()

# Module extension for envoy_dependencies
# DEPRECATED: Use //bazel/extensions:core.bzl instead
dependencies = module_extension(
    implementation = _dependencies_impl,
    doc = """
    DEPRECATED: Extension for Envoy's main dependencies.
    
    This extension has been consolidated into //bazel/extensions:core.bzl
    for better maintainability and reduced extension proliferation.
    
    Please migrate to the consolidated core extension.
    """,
)

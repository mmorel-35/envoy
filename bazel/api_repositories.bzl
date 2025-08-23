load("@envoy_api//bazel:repositories.bzl", "api_dependencies")

def envoy_api_dependencies():
    api_dependencies()

def _envoy_api_dependencies_impl(module_ctx):
    """Implementation of the envoy_api_dependencies extension."""
    # Call the main api dependencies function
    envoy_api_dependencies()

# Module extension for envoy_api_dependencies following consistent naming convention
envoy_api_dependencies_main_ext = module_extension(
    implementation = _envoy_api_dependencies_impl,
    doc = """
    Extension for Envoy's main API dependencies.
    
    This extension wraps the envoy_api_dependencies() function to make it
    available as a bzlmod module extension, following the consistent
    naming convention envoy_*_ext across all Envoy modules.
    
    Note: This is distinct from envoy_api_dependencies_ext in api/bazel/repositories.bzl
    which handles the API submodule dependencies directly.
    """,
)

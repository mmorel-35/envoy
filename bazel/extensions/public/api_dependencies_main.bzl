"""Public extension for Envoy's main API dependencies wrapper."""

load("//bazel:api_repositories.bzl", "envoy_api_dependencies")

def _api_dependencies_main_impl(module_ctx):
    """Implementation for api_dependencies_main extension.
    
    This extension wraps the envoy_api_dependencies() function to make it
    available as a bzlmod module extension.
    """
    # Call the main API dependencies wrapper function
    envoy_api_dependencies()

# Module extension for envoy_api_dependencies
api_dependencies_main = module_extension(
    implementation = _api_dependencies_main_impl,
    doc = """
    Extension for Envoy's main API dependencies wrapper.
    
    This extension wraps the envoy_api_dependencies() function to make it
    available as a bzlmod module extension. This is distinct from the
    api_dependencies extension which handles the API submodule dependencies directly.
    """,
)
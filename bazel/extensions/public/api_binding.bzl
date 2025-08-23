"""Public extension for Envoy API binding."""

load("//bazel:api_binding.bzl", "envoy_api_binding")

def _api_binding_impl(module_ctx):
    """Implementation for api_binding extension.
    
    This extension wraps the envoy_api_binding() function to make it
    available as a bzlmod module extension.
    """
    # Call the API binding function
    envoy_api_binding()

# Module extension for envoy_api_binding
api_binding = module_extension(
    implementation = _api_binding_impl,
    doc = """
    Extension for Envoy API binding.
    
    This extension wraps the envoy_api_binding() function to make it
    available as a bzlmod module extension, handling API repository 
    binding and setup.
    """,
)
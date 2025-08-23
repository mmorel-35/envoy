"""Extension for Envoy's extra dependencies."""

load("//bazel:repositories_extra.bzl", "envoy_dependencies_extra")

def _dependencies_extra_impl(module_ctx):
    """Implementation for dependencies_extra extension.
    
    This extension wraps the envoy_dependencies_extra() function to make it
    available as a bzlmod module extension.
    """
    # Call the extra dependencies function
    envoy_dependencies_extra()

# Module extension for envoy_dependencies_extra
dependencies_extra = module_extension(
    implementation = _dependencies_extra_impl,
    doc = """
    Extension for Envoy's extra dependencies.
    
    This extension wraps the envoy_dependencies_extra() function to make it
    available as a bzlmod module extension. These are dependencies that rely  
    on a first stage of dependency loading in envoy_dependencies().
    """,
)
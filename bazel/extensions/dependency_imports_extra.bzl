"""Extension for Envoy's extra dependency imports."""

load("//bazel:dependency_imports_extra.bzl", "envoy_dependency_imports_extra")

def _dependency_imports_extra_impl(module_ctx):
    """Implementation for dependency_imports_extra extension.

    This extension wraps the envoy_dependency_imports_extra() function to make it
    available as a bzlmod module extension.
    """

    # Call the extra dependency imports function
    envoy_dependency_imports_extra()

# Module extension for envoy_dependency_imports_extra
dependency_imports_extra = module_extension(
    implementation = _dependency_imports_extra_impl,
    doc = """
    Extension for Envoy's extra dependency imports.
    
    This extension wraps the envoy_dependency_imports_extra() function to make it
    available as a bzlmod module extension, handling additional dependency 
    imports and configurations.
    """,
)

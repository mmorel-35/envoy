"""Extension for Envoy's dependency imports."""

load("//bazel:dependency_imports.bzl", "envoy_dependency_imports")

def _dependency_imports_impl(module_ctx):
    """Implementation for dependency_imports extension.
    
    This extension wraps the envoy_dependency_imports() function to make it
    available as a bzlmod module extension.
    """
    # Call the dependency imports function
    envoy_dependency_imports()

# Module extension for envoy_dependency_imports
dependency_imports = module_extension(
    implementation = _dependency_imports_impl,
    doc = """
    Extension for Envoy's dependency imports.
    
    This extension wraps the envoy_dependency_imports() function to make it
    available as a bzlmod module extension, handling toolchain imports 
    and registrations.
    """,
)
load("@dynamic_modules_rust_sdk_crate_index//:defs.bzl", "crate_repositories")

# Dependencies that rely on a first stage of envoy_dependency_imports() in dependency_imports.bzl.
def envoy_dependency_imports_extra():
    crate_repositories()

def _envoy_dependency_imports_extra_impl(module_ctx):
    """Implementation of the envoy_dependency_imports_extra extension."""
    # Call the extra dependency imports function
    envoy_dependency_imports_extra()

# Module extension for envoy_dependency_imports_extra following consistent naming convention
envoy_dependency_imports_extra_ext = module_extension(
    implementation = _envoy_dependency_imports_extra_impl,
    doc = """
    Extension for Envoy's extra dependency imports.
    
    This extension wraps the envoy_dependency_imports_extra() function to make it
    available as a bzlmod module extension, following the consistent
    naming convention envoy_*_ext across all Envoy modules.
    """,
)

"""Extension for Envoy's dependency imports."""

load("@rules_fuzzing//fuzzing:repositories.bzl", "rules_fuzzing_dependencies")
load("@envoy_toolshed//coverage/grcov:grcov_repository.bzl", "grcov_repository")

def _dependency_imports_impl(module_ctx):
    """Implementation for dependency_imports extension.

    This extension wraps the envoy_dependency_imports() function to make it
    available as a bzlmod module extension.
    """

    # Call the dependency imports function
    rules_fuzzing_dependencies(
        oss_fuzz = True,
        honggfuzz = False,
    )
    grcov_repository()

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

"""Extension for Envoy's dependency imports.

DEPRECATED: This extension has been consolidated into toolchains.bzl for better maintainability.
Please use //bazel/extensions:toolchains.bzl instead.

See BZLMOD_RECOMMENDATIONS.md for details on the consolidation effort.
"""

load("@rules_fuzzing//fuzzing:repositories.bzl", "rules_fuzzing_dependencies")
load("@envoy_toolshed//coverage/grcov:grcov_repository.bzl", "grcov_repository")

def _dependency_imports_impl(module_ctx):
    """Implementation for dependency_imports extension.

    DEPRECATED: Use //bazel/extensions:toolchains.bzl instead.

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
# DEPRECATED: Use //bazel/extensions:toolchains.bzl instead
dependency_imports = module_extension(
    implementation = _dependency_imports_impl,
    doc = """
    DEPRECATED: Extension for Envoy's dependency imports.
    
    This extension has been consolidated into //bazel/extensions:toolchains.bzl
    for better maintainability and reduced extension proliferation.
    
    Please migrate to the consolidated toolchains extension.
    """,
)

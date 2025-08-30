"""Extension for Envoy's extra dependency imports.

DEPRECATED: This extension has been consolidated into toolchains.bzl for better maintainability.
Please use //bazel/extensions:toolchains.bzl instead.

See BZLMOD_RECOMMENDATIONS.md for details on the consolidation effort.
"""

load("//bazel:dependency_imports_extra.bzl", "envoy_dependency_imports_extra")

def _dependency_imports_extra_impl(module_ctx):
    """Implementation for dependency_imports_extra extension.

    DEPRECATED: Use //bazel/extensions:toolchains.bzl instead.

    This extension wraps the envoy_dependency_imports_extra() function to make it
    available as a bzlmod module extension.
    """

    # Call the extra dependency imports function
    envoy_dependency_imports_extra()

# Module extension for envoy_dependency_imports_extra
# DEPRECATED: Use //bazel/extensions:toolchains.bzl instead
dependency_imports_extra = module_extension(
    implementation = _dependency_imports_extra_impl,
    doc = """
    DEPRECATED: Extension for Envoy's extra dependency imports.
    
    This extension has been consolidated into //bazel/extensions:toolchains.bzl
    for better maintainability and reduced extension proliferation.
    
    Please migrate to the consolidated toolchains extension.
    """,
)

"""Extension for Envoy repository setup.

DEPRECATED: This extension has been consolidated into toolchains.bzl for better maintainability.
Please use //bazel/extensions:toolchains.bzl instead.

See BZLMOD_RECOMMENDATIONS.md for details on the consolidation effort.
"""

load("//bazel:repo.bzl", "envoy_repo")

def _repo_impl(module_ctx):
    """Implementation for repo extension.

    DEPRECATED: Use //bazel/extensions:toolchains.bzl instead.

    This extension wraps the envoy_repo() function to make it
    available as a bzlmod module extension.
    """

    # Call the repo setup function
    envoy_repo()

# Module extension for envoy_repo
# DEPRECATED: Use //bazel/extensions:toolchains.bzl instead
repo = module_extension(
    implementation = _repo_impl,
    doc = """
    DEPRECATED: Extension for Envoy repository setup.
    
    This extension has been consolidated into //bazel/extensions:toolchains.bzl
    for better maintainability and reduced extension proliferation.
    
    Please migrate to the consolidated toolchains extension.
    """,
)

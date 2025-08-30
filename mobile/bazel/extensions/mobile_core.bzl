"""Consolidated core extension for Envoy Mobile dependencies and repositories.

This extension consolidates the following mobile functionality:
- Mobile dependencies (mobile.bzl)
- Mobile repositories (repos.bzl)
- API dependencies from envoy_api module
"""

load("//bazel:envoy_mobile_dependencies.bzl", "envoy_mobile_dependencies")
load("//bazel:envoy_mobile_repositories.bzl", "envoy_mobile_repositories")

def _mobile_core_impl(module_ctx):
    """Implementation for mobile_core extension.

    This extension consolidates mobile dependencies and repository setup
    into a single extension for better maintainability.
    """

    # Call the mobile dependencies function
    envoy_mobile_dependencies()
    
    # Call the mobile repositories function
    envoy_mobile_repositories()

# Module extension for consolidated mobile core functionality
mobile_core = module_extension(
    implementation = _mobile_core_impl,
    doc = """
    Consolidated extension for Envoy Mobile core dependencies and repositories.
    
    This extension combines mobile dependencies and repository setup into a
    single streamlined extension, following bzlmod best practices for
    reducing extension complexity.
    """,
)
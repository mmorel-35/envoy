"""Extension for Envoy Mobile repositories."""

load("//bazel:envoy_mobile_repositories.bzl", "envoy_mobile_repositories")

def _envoy_mobile_repos_impl(module_ctx):
    """Implementation for envoy_mobile_repos extension.
    
    This extension wraps the envoy_mobile_repositories() function to make it
    available as a bzlmod module extension.
    """
    # Call the mobile repositories function
    envoy_mobile_repositories()

# Module extension for envoy_mobile_repositories
repos = module_extension(
    implementation = _envoy_mobile_repos_impl,
    doc = """
    Extension for Envoy Mobile repositories.
    
    This extension wraps the envoy_mobile_repositories() function to make it
    available as a bzlmod module extension, handling mobile-specific
    repository setup and configuration.
    """,
)
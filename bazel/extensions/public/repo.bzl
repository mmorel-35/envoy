"""Public extension for Envoy repository setup."""

load("//bazel:repo.bzl", "envoy_repo")

def _repo_impl(module_ctx):
    """Implementation for repo extension.
    
    This extension wraps the envoy_repo() function to make it
    available as a bzlmod module extension.
    """
    # Call the repo setup function
    envoy_repo()

# Module extension for envoy_repo
repo = module_extension(
    implementation = _repo_impl,
    doc = """
    Extension for Envoy repository setup.
    
    This extension wraps the envoy_repo() function to make it
    available as a bzlmod module extension, handling repository 
    metadata and tooling setup.
    """,
)
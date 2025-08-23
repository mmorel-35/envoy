"""Extension for Envoy Mobile workspace setup."""

load("//bazel:envoy_mobile_workspace.bzl", "envoy_mobile_workspace")

def _envoy_mobile_workspace_impl(module_ctx):
    """Implementation for envoy_mobile_workspace extension.
    
    This extension wraps the envoy_mobile_workspace() function to make it
    available as a bzlmod module extension.
    """
    # Call the mobile workspace function
    envoy_mobile_workspace()

# Module extension for envoy_mobile_workspace
envoy_mobile_workspace = module_extension(
    implementation = _envoy_mobile_workspace_impl,
    doc = """
    Extension for Envoy Mobile workspace setup.
    
    This extension wraps the envoy_mobile_workspace() function to make it
    available as a bzlmod module extension, handling Xcode and 
    provisioning profile setup.
    """,
)
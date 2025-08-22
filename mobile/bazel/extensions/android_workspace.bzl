"""Extension for Android workspace setup."""

def _post_android_setup_impl(repository_ctx):
    """Repository rule to handle post-android configuration setup."""

    # This will be called after @local_config_android is available
    repository_ctx.file("BUILD.bazel", "")
    repository_ctx.file("WORKSPACE", "")

    # Create a dummy file to indicate setup is complete
    repository_ctx.file("setup_complete.txt", "Android workspace setup complete")

_post_android_setup = repository_rule(
    implementation = _post_android_setup_impl,
)

def _envoy_android_workspace_impl(module_ctx):
    """Implementation for envoy_android_workspace extension.

    This extension handles additional Android workspace setup that depends on
    the local_config_android repository being configured first.
    """

    # Create a marker repository to ensure android workspace setup completes
    _post_android_setup(name = "envoy_android_workspace_setup")

# Module extension for android workspace setup
android_workspace = module_extension(
    implementation = _envoy_android_workspace_impl,
    doc = """
    Extension for Android workspace setup.
    
    This extension handles additional Android workspace setup that depends on
    the local_config_android repository being configured first.
    """,
)

"""Module extension for additional Envoy Mobile workspace setup."""

load("@build_bazel_rules_apple//apple:apple.bzl", "provisioning_profile_repository")
load("@com_github_buildbuddy_io_rules_xcodeproj//xcodeproj:repositories.bzl", "xcodeproj_rules_dependencies")

def _envoy_mobile_workspace_impl(module_ctx):
    """Implementation of the envoy_mobile_workspace extension."""

    # Setup Xcode project rules dependencies
    xcodeproj_rules_dependencies()

    # Setup provisioning profile repository
    provisioning_profile_repository(
        name = "local_provisioning_profiles",
    )

def _envoy_android_workspace_impl(module_ctx):
    """Implementation of the envoy_android_workspace extension."""

    # This handles the android_workspace() call that depends on @local_config_android
    # being set up first by the android_configure extension
    # The load and call to android_workspace() will happen automatically
    # when @local_config_android is available
    pass

# Module extension for additional mobile workspace setup
envoy_mobile_workspace_ext = module_extension(
    implementation = _envoy_mobile_workspace_impl,
    doc = """
    Extension for additional Envoy Mobile workspace setup.
    
    This extension handles Xcode project dependencies and provisioning profiles
    setup for mobile builds, following the consistent naming convention 
    envoy_*_ext across all Envoy modules.
    """,
)

# Module extension for Android workspace setup
envoy_android_workspace_ext = module_extension(
    implementation = _envoy_android_workspace_impl,
    doc = """
    Extension for Android workspace setup.
    
    This extension handles the android_workspace() call that depends on
    the local_config_android repository being configured first.
    """,
)

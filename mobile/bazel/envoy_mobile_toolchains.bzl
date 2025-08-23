load("@rules_detekt//detekt:toolchains.bzl", "rules_detekt_toolchains")
load("@rules_java//java:repositories.bzl", "rules_java_toolchains")
load("@rules_kotlin//kotlin:core.bzl", "kt_register_toolchains")
load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_toolchains")

def envoy_mobile_toolchains():
    rules_java_toolchains()
    kt_register_toolchains()
    rules_detekt_toolchains()
    rules_proto_grpc_toolchains()

def _envoy_mobile_toolchains_impl(module_ctx):
    """Implementation of the envoy_mobile_toolchains extension."""
    # Call the mobile toolchains function
    envoy_mobile_toolchains()

# Module extension for envoy_mobile_toolchains following consistent naming convention
envoy_mobile_toolchains_ext = module_extension(
    implementation = _envoy_mobile_toolchains_impl,
    doc = """
    Extension for Envoy Mobile's toolchain setup.
    
    This extension wraps the envoy_mobile_toolchains() function to make it
    available as a bzlmod module extension, following the consistent
    naming convention envoy_*_ext across all Envoy modules.
    """,
)

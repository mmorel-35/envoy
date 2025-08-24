# Native binding wrapper for bzlmod compatibility
# 
# This wrapper provides a compatibility layer for Envoy's native bindings migration to bzlmod.
# It ensures native bindings are only executed in non-bzlmod (legacy WORKSPACE) builds.
# For bzlmod builds, bindings are skipped with clear logging and migration guidance.
#
# Usage:
#   load(":native_binding_wrapper.bzl", "envoy_native_bind")
#
#   # Individual binding
#   envoy_native_bind(name = "ssl", actual = "@envoy//bazel:boringssl")
#   envoy_native_bind(name = "protobuf", actual = "@com_google_protobuf//:protobuf")
#   envoy_native_bind(name = "grpc", actual = "@com_github_grpc_grpc//:grpc++")
#
# Migration Path:
#   1. WORKSPACE builds: Execute native bindings normally (backward compatible)
#   2. bzlmod builds: Skip bindings with guidance to use //third_party compatibility layer
#   3. Future: Remove bindings entirely and use direct @repo//:target dependencies

# Bzlmod context detection - in bzlmod, labels start with @@
_IS_BZLMOD = str(Label("//:invalid")).startswith("@@")

def envoy_native_bind(name, actual = None, **kwargs):
    """
    Wrapper for native.bind() that provides bzlmod compatibility.
    
    This function ensures native bindings are only executed in non-bzlmod (legacy WORKSPACE) builds.
    For bzlmod builds, the wrapper skips native bindings with a clear warning.
    
    Args:
        name: The bind name
        actual: The target being bound
        **kwargs: Additional arguments passed to native.bind()
    """
    if not _IS_BZLMOD:
        # Legacy WORKSPACE mode - execute native binding
        native.bind(name = name, actual = actual, **kwargs)
    else:
        # bzlmod mode - skip native binding with warning
        # Note: In bzlmod mode, native bindings are not supported.
        # Dependencies should be accessed directly via @repo//:target syntax
        # or through the //third_party compatibility layer.
        print("WARNING: Skipping native.bind(name='{}', actual='{}') in bzlmod mode. ".format(name, actual) +
              "Use direct @repo//:target references or //third_party:{} alias instead.".format(name))
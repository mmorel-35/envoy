# Native binding wrapper for bzlmod compatibility
# This wrapper ensures native bindings are only executed in non-bzlmod (legacy WORKSPACE) builds.
# For bzlmod builds, bindings are skipped with clear logging.

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

def envoy_conditional_native_bind_group(bindings):
    """
    Apply native bindings conditionally based on bzlmod context.
    
    This function provides a clean way to apply multiple native bindings
    that should all be skipped in bzlmod mode.
    
    Args:
        bindings: List of dictionaries with 'name' and 'actual' keys
    """
    if not _IS_BZLMOD:
        # Legacy WORKSPACE mode - execute all native bindings
        for binding in bindings:
            native.bind(name = binding["name"], actual = binding["actual"])
    else:
        # bzlmod mode - log skipped bindings
        binding_names = [b["name"] for b in bindings]
        print("INFO: Skipping {} native bindings in bzlmod mode: {}. ".format(
            len(bindings), ", ".join(binding_names)) +
              "Use direct @repo//:target references or //third_party compatibility layer instead.")
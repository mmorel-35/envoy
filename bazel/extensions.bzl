"""Module extensions for Envoy's non-module dependencies.

This file defines module extensions to support Envoy's bzlmod migration while
respecting existing WORKSPACE patches and custom BUILD files.

## Background

Bazel's new module system (bzlmod) allows for better dependency management, but
requires a migration from the traditional WORKSPACE-based approach. This extension
serves as a bridge during the migration.

## Implementation Pattern

Following the pattern established in api/bazel/extensions.bzl, this extension:
1. Calls envoy_dependencies(bzlmod=True) from bazel/repositories.bzl
2. The same function is used by WORKSPACE mode (with bzlmod=False)
3. When bzlmod=True, dependencies already in BCR are skipped via conditional checks

This approach:
- Avoids code duplication between WORKSPACE and bzlmod modes
- Maintains a single source of truth for dependency definitions
- Ensures both modes load dependencies identically
- Makes migration clear and reviewable

## Usage in MODULE.bazel

This extension is used in the root MODULE.bazel file as follows:

```python
envoy_deps = use_extension("//bazel:extensions.bzl", "envoy_dependencies")
use_repo(
    envoy_deps,
    "boringssl_fips",
    "com_github_grpc_grpc",
    # ... other non-module dependencies
)
```

## Maintenance

When adding new dependencies:
- Add them to envoy_dependencies() in bazel/repositories.bzl
- If the dependency is in BCR, wrap it with `if not bzlmod:`
- If patches are needed, add them to the function
- Update the use_repo() call in MODULE.bazel

When removing dependencies:
- If moving to BCR, wrap existing calls with `if not bzlmod:`
- Update the use_repo() call in MODULE.bazel
- Document the change

## Bzlmod Migration Status

Dependencies already migrated to BCR (skipped when bzlmod=True):
- bazel_features, highway, fast_float
- zlib, zstd, org_brotli, re2
- protobuf, spdlog, fmt, yaml-cpp
- nlohmann_json, xxhash, gperftools, numactl
- flatbuffers, google_benchmark, googletest
- bazel_gazelle, io_bazel_rules_go
- platforms, rules_shell, rules_cc, rules_foreign_cc
- boringssl (non-FIPS), emsdk
- rules_fuzzing, rules_license, rules_pkg, rules_shellcheck
- aspect_bazel_lib, abseil-cpp

See MODULE.bazel for the complete list of bazel_dep() entries.
"""

load(":repositories.bzl", "envoy_dependencies")

def _envoy_dependencies_impl(module_ctx):
    """Implementation of the envoy_dependencies module extension.
    
    This extension calls envoy_dependencies(bzlmod=True) which loads all Envoy
    dependencies. Dependencies already available in BCR are skipped via conditional
    checks within the function.
    
    Args:
        module_ctx: The module extension context
    """
    envoy_dependencies(bzlmod = True)

# Define the module extension
envoy_dependencies_extension = module_extension(
    implementation = _envoy_dependencies_impl,
    doc = """
    Extension for Envoy dependencies not available in BCR or requiring patches.

    This extension calls the same envoy_dependencies() function used by WORKSPACE mode,
    but with bzlmod=True. This ensures both build systems load dependencies identically,
    making the migration clear and reviewable.
    
    Dependencies already in BCR are skipped automatically via conditional checks.
    For WORKSPACE mode, call envoy_dependencies() directly from WORKSPACE.
    
    See the module documentation above for maintenance guidelines.
    """,
)

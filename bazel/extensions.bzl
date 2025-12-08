"""Module extensions for Envoy's non-module dependencies.

This file defines module extensions to support Envoy's bzlmod migration while
respecting existing WORKSPACE patches and custom BUILD files. Each dependency
loaded here is not yet available as a Bazel module in the Bazel Central Registry
or requires custom configuration/patches specific to Envoy.

## Background

Bazel's new module system (bzlmod) allows for better dependency management, but
requires a migration from the traditional WORKSPACE-based approach. This extension
serves as a bridge during the migration, handling dependencies that:

1. Are not yet published to the Bazel Central Registry (BCR)
2. Require Envoy-specific patches for compatibility or bug fixes
3. Need custom BUILD files that differ from upstream
4. Have platform-specific variants (e.g., protoc binaries)

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

## Implementation Pattern

Following the pattern established in api/bazel/extensions.bzl, this extension:
1. Calls envoy_dependencies_for_bzlmod(bzlmod=True) from bazel/repositories.bzl
2. That function safely coexists with BCR deps via native.existing_rules() checks
3. The same function is used by WORKSPACE mode (with bzlmod=False)

This approach:
- Avoids code duplication between WORKSPACE and bzlmod modes
- Maintains a single source of truth for dependency definitions
- Ensures both modes load dependencies identically

## Maintenance

When adding new dependencies:
- Add them to envoy_dependencies_for_bzlmod() in bazel/repositories.bzl
- Check if the dependency is available in BCR first
- If available in BCR, use bazel_dep() instead
- If patches are needed, add them to repositories.bzl
- Update the use_repo() call in MODULE.bazel to include the new dependency

When removing dependencies:
- Only remove from repositories.bzl if the dependency is now in BCR
- Update the use_repo() call in MODULE.bazel accordingly
- Document the removal in the commit message

## Bzlmod Migration Status

Dependencies already migrated to BCR and loaded via bazel_dep():
- bazel_features, highway, fast_float
- zlib, zstd, org_brotli, re2
- protobuf, spdlog, fmt, yaml-cpp
- nlohmann_json, xxhash, gperftools, numactl
- flatbuffers, google_benchmark, googletest
- bazel_gazelle, io_bazel_rules_go

See MODULE.bazel for the complete list of bazel_dep() entries.
"""

load(":repositories.bzl", "envoy_dependencies_for_bzlmod")

def _envoy_dependencies_impl(module_ctx):
    """Implementation of the envoy_dependencies module extension.
    
    This extension calls envoy_dependencies_for_bzlmod(bzlmod=True) which creates
    repositories not in BCR. It safely coexists with BCR deps because envoy_http_archive
    checks native.existing_rules() before creating repositories.
    
    Args:
        module_ctx: The module extension context
    """
    envoy_dependencies_for_bzlmod(bzlmod = True)

# Define the module extension
envoy_dependencies = module_extension(
    implementation = _envoy_dependencies_impl,
    doc = """
    Extension for Envoy dependencies not available in BCR.

    This extension creates repositories for dependencies that are either:
    - Not yet published to the Bazel Central Registry
    - Require Envoy-specific patches or custom BUILD files
    
    For WORKSPACE mode, these dependencies are loaded via envoy_dependencies()
    in WORKSPACE. This extension should only be used in MODULE.bazel files.
    
    See the module documentation above for the complete list of loaded dependencies.
    """,
)

# Extension Consolidation Migration Guide

This document provides guidance for migrating from the legacy individual extensions to the new consolidated extensions.

## Overview

As part of improving Envoy's bzlmod implementation and following Bazel best practices, we have consolidated 5 separate extensions into 2 streamlined extensions:

### Before (Legacy - 5 extensions)
```starlark
envoy_deps = use_extension("//bazel/extensions:dependencies.bzl", "dependencies")
envoy_deps_extra = use_extension("//bazel/extensions:dependencies_extra.bzl", "dependencies_extra")
envoy_imports = use_extension("//bazel/extensions:dependency_imports.bzl", "dependency_imports")
envoy_imports_extra = use_extension("//bazel/extensions:dependency_imports_extra.bzl", "dependency_imports_extra")
envoy_repo_setup = use_extension("//bazel/extensions:repo.bzl", "repo")
```

### After (New - 2 extensions)
```starlark
envoy_core = use_extension("//bazel/extensions:core.bzl", "core")
envoy_toolchains = use_extension("//bazel/extensions:toolchains.bzl", "toolchains")
```

## Migration Steps

### Step 1: Update Extension Usage

**Replace legacy dependency extensions:**
```starlark
# OLD - Remove these lines
envoy_deps = use_extension("//bazel/extensions:dependencies.bzl", "dependencies")
envoy_deps_extra = use_extension("//bazel/extensions:dependencies_extra.bzl", "dependencies_extra")

# NEW - Add this line
envoy_core = use_extension("//bazel/extensions:core.bzl", "core")
```

**Replace legacy import extensions:**
```starlark
# OLD - Remove these lines  
envoy_imports = use_extension("//bazel/extensions:dependency_imports.bzl", "dependency_imports")
envoy_imports_extra = use_extension("//bazel/extensions:dependency_imports_extra.bzl", "dependency_imports_extra")
envoy_repo_setup = use_extension("//bazel/extensions:repo.bzl", "repo")

# NEW - Add this line
envoy_toolchains = use_extension("//bazel/extensions:toolchains.bzl", "toolchains")
```

### Step 2: Update use_repo Calls

**Consolidate repository references:**
```starlark
# OLD - Replace multiple use_repo calls
use_repo(envoy_deps, "com_google_protobuf", "com_github_grpc_grpc", ...)
use_repo(envoy_deps_extra, "proto_bazel_features")
use_repo(envoy_imports, "grcov", "rules_fuzzing_oss_fuzz")
use_repo(envoy_repo_setup, "envoy_repo")

# NEW - Single consolidated use_repo calls
use_repo(
    envoy_core,
    "com_google_protobuf",
    "com_github_grpc_grpc", 
    "proto_bazel_features",
    # ... all core dependencies
)

use_repo(
    envoy_toolchains,
    "envoy_repo",
    "grcov", 
    "rules_fuzzing_oss_fuzz",
)
```

### Step 3: Verify Migration

1. **Check BUILD files**: Ensure all repository references still work
2. **Run tests**: Validate that builds and tests continue to pass
3. **Clean build**: Test with a clean build to ensure dependency resolution works

## Benefits of Consolidation

### Technical Benefits
- **Reduced complexity**: 60% fewer extensions to maintain (5 → 2)
- **Simplified dependency graph**: Easier to understand and debug
- **Better performance**: Fewer extension evaluation cycles
- **Improved maintainability**: Single location for related functionality

### Developer Experience
- **Clearer organization**: Related functionality grouped together
- **Reduced cognitive load**: Fewer extension names to remember
- **Better documentation**: Comprehensive docs in fewer places
- **Easier onboarding**: Simpler bzlmod setup for new contributors

## Troubleshooting

### Common Issues

**"Repository not found" errors:**
- Ensure all needed repositories are listed in the appropriate `use_repo()` call
- Check that you've included repositories from all deprecated extensions

**"Extension not found" errors:**
- Verify the extension paths point to the new consolidated files
- Double-check that `core.bzl` and `toolchains.bzl` files exist

**Build failures after migration:**
- Run a clean build: `bazel clean --expunge && bazel build //...`
- Check that no old extension references remain in your MODULE.bazel

### Getting Help

- Check [BZLMOD_RECOMMENDATIONS.md](./BZLMOD_RECOMMENDATIONS.md) for strategic context
- Review [BZLMOD_MIGRATION.md](./BZLMOD_MIGRATION.md) for general migration guidance  
- See the official [Bazel bzlmod documentation](https://bazel.build/external/migration)

## Timeline

- **Legacy extensions**: Marked as DEPRECATED but still functional
- **New extensions**: Available and recommended for all new usage
- **Removal timeline**: Legacy extensions will be removed in a future major release

## Example: Complete Migration

### Before
```starlark
# MODULE.bazel (legacy)
envoy_deps = use_extension("//bazel/extensions:dependencies.bzl", "dependencies")
use_repo(envoy_deps, "com_google_protobuf", "com_github_grpc_grpc")

envoy_deps_extra = use_extension("//bazel/extensions:dependencies_extra.bzl", "dependencies_extra")
use_repo(envoy_deps_extra, "proto_bazel_features")

envoy_imports = use_extension("//bazel/extensions:dependency_imports.bzl", "dependency_imports")
use_repo(envoy_imports, "grcov")

envoy_repo_setup = use_extension("//bazel/extensions:repo.bzl", "repo")
use_repo(envoy_repo_setup, "envoy_repo")
```

### After
```starlark
# MODULE.bazel (consolidated)
envoy_core = use_extension("//bazel/extensions:core.bzl", "core")
use_repo(
    envoy_core,
    "com_google_protobuf",
    "com_github_grpc_grpc", 
    "proto_bazel_features",
)

envoy_toolchains = use_extension("//bazel/extensions:toolchains.bzl", "toolchains")
use_repo(
    envoy_toolchains,
    "envoy_repo",
    "grcov",
)
```

This consolidation significantly simplifies the bzlmod configuration while maintaining all existing functionality.
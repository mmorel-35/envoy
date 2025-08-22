# Bzlmod Migration for Envoy

This document describes the minimal, functional bzlmod migration that resolves dependency cycles while maintaining full backward compatibility.

## Overview

This migration implements a **conservative, production-ready bzlmod setup** that addresses the `envoy_examples` cycle dependency issue while establishing a foundation for future dependency migrations.

## Migration Status

### ✅ Currently Migrated to MODULE.bazel

The following dependencies have been successfully migrated and are now managed across multiple MODULE.bazel files:

#### Core Dependencies (Root MODULE.bazel)
- `platforms` (1.0.0) - Platform definitions
- `bazel_features` (1.33.0) - Bazel feature detection
- `abseil-cpp` (20250512.1) - Abseil C++ library (exact version from repository_locations.bzl)
- `protobuf` (29.3) - Protocol buffers runtime (exact version from repository_locations.bzl)
- `rules_cc` (0.1.4) - C++ build rules (exact version from repository_locations.bzl)
- `rules_proto` (7.0.2) - Protocol buffer rules

#### Local Path Overrides
- `envoy_api` → `api/` - Envoy API definitions
- `envoy_build_config` → `mobile/envoy_build_config/` - Build configuration

### Multi-Component Architecture

```
├── MODULE.bazel                              # Root Envoy dependencies
├── WORKSPACE.bzlmod                          # Hybrid mode with skip_targets
├── api/
│   └── MODULE.bazel                          # API module (protobuf, rules_proto)
├── mobile/
│   ├── MODULE.bazel                          # Mobile dependencies
│   ├── WORKSPACE.bzlmod                      # Mobile hybrid mode
│   └── envoy_build_config/
│       ├── MODULE.bazel                      # Build config (rules_cc)
│       └── WORKSPACE.bzlmod                  # Config hybrid mode
```

### 🔧 Key Fix: Cycle Dependency Resolution

**Problem Solved**: The CI was failing with:
```
ERROR: Failed to load Starlark extension '@@envoy_examples//bazel:env.bzl'.
Cycle in the workspace file detected.
```

**Solution**: 
- Modified `repositories.bzl` to support `skip_targets` parameter
- WORKSPACE.bzlmod now skips dependencies managed by MODULE.bazel
- Proper dependency isolation prevents conflicts

### ⏳ Remaining in WORKSPACE

Following user guidance to be minimal, the following dependencies remain in WORKSPACE:

#### Dependencies Requiring Patches (Not in BCR)
- **gRPC** - Complex dependency with Envoy-specific patches
- **boringssl** - Custom Envoy integration and FIPS variants
- **envoy_examples** - Example configurations
- **rules_foreign_cc** - Custom patches for Envoy builds

#### Platform-Specific Rules  
- **rules_apple** - iOS/macOS build rules
- **rules_android** - Android build rules
- **rules_fuzzing** - Fuzzing test rules

## Current Configuration

### Exact Version Matching
All migrated dependencies use **exactly the same versions** as `bazel/repository_locations.bzl`:
- Ensures zero regression in dependency resolution
- Maintains compatibility with existing CI/CD pipelines  
- Prevents version drift between WORKSPACE and MODULE.bazel

### Hybrid Mode Implementation
```python
# WORKSPACE.bzlmod
envoy_dependencies(skip_targets = [
    "rules_cc",           # Managed by MODULE.bazel
    "com_google_protobuf", # Managed by MODULE.bazel (protobuf)
    "com_google_absl",    # Managed by MODULE.bazel (abseil-cpp)
])
```

### Enhanced repositories.bzl
Added bzlmod compatibility:
```python
def envoy_dependencies(skip_targets = []):
    # Skip dependencies managed by MODULE.bazel
    if "rules_cc" not in skip_targets:
        external_http_archive("rules_cc")
    if "com_google_protobuf" not in skip_targets:
        _com_google_protobuf()
    # ... etc
```

## Current Configuration

### Hybrid Mode
The repository now operates in hybrid mode:
- `--enable_bzlmod` is set in `.bazelrc`
- Both `WORKSPACE` and `MODULE.bazel` are active
- Bzlmod dependencies take precedence over WORKSPACE for the same targets

### Version Alignment
All migrated dependencies use versions that match or are compatible with the existing WORKSPACE versions to ensure build compatibility.

## Migration Best Practices

### 1. Gradual Migration
- Migrate stable, well-supported dependencies first
- Keep complex dependency trees in WORKSPACE initially
- Test thoroughly after each migration batch

### 2. Version Pinning
- Use exact versions in MODULE.bazel for reproducibility
- Regularly update to newer compatible versions
- Coordinate updates with security patches

### 3. Dependency Resolution
- Bzlmod uses semantic versioning for dependency resolution
- Conflicts are resolved automatically using highest compatible version
- Use `bazel mod graph` to visualize dependency tree

### 4. Testing Strategy
- Test critical build paths after migration
- Verify that both local and CI builds work
- Check that mobile builds (iOS/Android) still function

## Next Steps for Maintainers

### Phase 2: gRPC Migration
```starlark
bazel_dep(name = "grpc", version = "1.69.0", repo_name = "com_github_grpc_grpc")
```

### Phase 3: Additional Core Libraries
```starlark
bazel_dep(name = "re2", version = "2024.12.01", repo_name = "com_googlesource_code_re2")
bazel_dep(name = "zlib", version = "1.3.1", repo_name = "net_zlib")
```

### Phase 4: Platform Rules
```starlark
bazel_dep(name = "rules_apple", version = "3.20.1", repo_name = "build_bazel_rules_apple")
bazel_dep(name = "rules_pkg", version = "1.1.0")
```

## Commands for Migration Testing

### Check dependency graph
```bash
bazel mod graph
```

### Validate MODULE.bazel
```bash
bazel mod show_extension_repos
```

### Test core builds
```bash
bazel build //source/common/...
bazel test //test/common/...
```

## Resources

- [Bazel Migration Guide](https://bazel.build/external/migration)
- [Bazel Central Registry](https://github.com/bazelbuild/bazel-central-registry)
- [Bzlmod User Guide](https://bazel.build/external/mod)
- [Module Resolution](https://bazel.build/external/mod#resolution)

## Troubleshooting

### Common Issues

1. **Duplicate dependency errors**: Remove conflicting entries from WORKSPACE
2. **Version conflicts**: Use `bazel mod graph` to identify conflicts
3. **Missing dependencies**: Check if dependency is available in BCR or needs custom repository rule

### Getting Help

- Check the [Bazel Community Slack](https://slack.bazel.build/) #bzlmod channel
- Review existing migration examples in the BCR
- Open issues in the [bazel-central-registry](https://github.com/bazelbuild/bazel-central-registry) for missing modules
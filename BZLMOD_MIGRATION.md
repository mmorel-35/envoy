# Envoy Bzlmod Migration

This document describes the conservative bzlmod migration implemented for Envoy to prepare for Bazel 8.0, which will drop WORKSPACE support, while preserving all custom patches and using automatic detection.

## Overview

This migration implements a conservative approach to prepare for Bazel 8.0's removal of WORKSPACE support. It only migrates dependencies **without patches** to MODULE.bazel. Dependencies requiring custom patches remain in the WORKSPACE.bzlmod/extensions system to preserve critical modifications.

## Problem Being Solved

Bazel 8.0 will drop support for the traditional WORKSPACE system, requiring migration to the MODULE.bazel (bzlmod) system. This migration must preserve all existing custom patches that Envoy requires for proper functionality.

## Solution: Conservative Patch-Preserving Migration with Automatic Detection

### Key Strategy
- **Only migrate clean dependencies** (no patches) to MODULE.bazel
- **Keep patched dependencies** in WORKSPACE.bzlmod/extensions system to preserve modifications
- **Automatic bzlmod detection** using built-in `native.existing_rules()` checks
- **Hybrid approach** ensures both bzlmod benefits and patch preservation

### Migration Criteria

**Dependencies migrated to MODULE.bazel:**
- No custom patches required
- Available in Bazel Central Registry (BCR) with compatible versions
- Standard build configuration (no special flags or build files)
- No complex repository rules or transformations needed

**Dependencies kept in WORKSPACE.bzlmod:**
- Require custom patches for Envoy compatibility
- Need special build files or patch commands
- Use complex repository configurations
- Have platform-specific or FIPS-related modifications
- Require specific versions not yet available in BCR

### Architecture

**1. MODULE.bazel - Clean Dependencies Only**
```python
# Minimal bzlmod migration - only dependencies without patches
bazel_dep(name = "bazel_features", version = "1.33.0")
bazel_dep(name = "fmt", version = "11.2.0")
bazel_dep(name = "google_benchmark", version = "1.9.4")
bazel_dep(name = "googleapis", version = "0.0.0-20241220-5e258e33.bcr.1")
bazel_dep(name = "platforms", version = "1.0.0") 
bazel_dep(name = "protoc-gen-validate", version = "1.2.1.bcr.1")
bazel_dep(name = "rules_python", version = "0.35.0")
bazel_dep(name = "rules_cc", version = "0.1.4")
bazel_dep(name = "rules_pkg", version = "1.1.0")
bazel_dep(name = "rules_shell", version = "0.5.1")
bazel_dep(name = "rules_shellcheck", version = "0.3.3")
bazel_dep(name = "spdlog", version = "1.15.3")
bazel_dep(name = "xxhash", version = "0.8.3")

# Local modules with overrides
bazel_dep(name = "envoy_api", version = "0.0.0-dev")
bazel_dep(name = "envoy_build_config", version = "0.0.0-dev")
local_path_override(module_name = "envoy_api", path = "api")
local_path_override(module_name = "envoy_build_config", path = "mobile/envoy_build_config")

# Google APIs extension for proper import handling
switched_rules = use_extension("@com_google_googleapis//:extensions.bzl", "switched_rules")
switched_rules.use_languages(cc = True, go = True, grpc = True, python = True)
use_repo(switched_rules, "com_google_googleapis_imports")
```

### Google APIs Extension

The googleapis dependency requires special configuration through the `switched_rules` extension. This extension automatically generates the appropriate repository rules based on which languages are enabled:

```python
switched_rules = use_extension("@com_google_googleapis//:extensions.bzl", "switched_rules")
switched_rules.use_languages(
    cc = True,      # Enable C++ protobuf generation
    go = True,      # Enable Go protobuf generation  
    grpc = True,    # Enable gRPC service generation
    python = True,  # Enable Python protobuf generation
)
use_repo(switched_rules, "com_google_googleapis_imports")
```

This replaces the manual `switched_rules_by_language` call that was previously in WORKSPACE.bzlmod, providing the same functionality through the bzlmod extension system.

**2. WORKSPACE.bzlmod - Patched Dependencies**
```python
# Loads all dependencies using automatic bzlmod detection
envoy_dependencies()  # Uses native.existing_rules() to skip MODULE.bazel deps
```

### Dependencies by Location

#### Migrated to MODULE.bazel (No Patches)
- **bazel_features** (1.33.0) - Essential platform detection
- **platforms** (1.0.0) - Platform configuration
- **google_benchmark** (1.9.4) - Performance benchmarking
- **googleapis** (0.0.0-20241220-5e258e33.bcr.1) - Google APIs with switched_rules extension
- **spdlog** (1.15.3) - Logging library  
- **fmt** (11.2.0) - String formatting
- **xxhash** (0.8.3) - Hash function library
- **protoc-gen-validate** (1.2.1.bcr.1) - Protocol buffer validation
- **rules_python** (0.35.0) - Python build rules
- **rules_cc** (0.1.4) - C++ build rules
- **rules_pkg** (1.1.0) - Package creation rules
- **rules_shell** (0.5.1) - Shell script build rules
- **rules_shellcheck** (0.3.3) - Shell script linting
- **envoy_api** - Internal API module (local override)
- **envoy_build_config** - Internal build config (local override)

#### Remaining in WORKSPACE.bzlmod (With Patches)
- **protobuf** - Extensive `protobuf.patch` for arena.h modifications
- **abseil-cpp** - Custom `abseil.patch` for compatibility
- **grpc** - Custom `grpc.patch` modifications
- **googletest** - Custom `googletest.patch`
- **boringssl** - FIPS support patches (`boringssl_fips.patch`)
- **rules_rust** - Platform-specific patches (crate configuration handled via WORKSPACE.bzlmod)
- **c-ares** - Custom patches for DNS resolution
- All other dependencies with patches or complex configurations

## Impact

- ✅ **Prepares for Bazel 8.0** - Establishes bzlmod foundation before WORKSPACE deprecation in Bazel 8.0
- ✅ **Preserves all patches** - No functionality regression, all custom modifications maintained
- ✅ **Conservative approach** - Minimal risk by only migrating dependencies without patches
- ✅ **Automatic detection** - Built-in bzlmod mode detection without requiring manual configuration
- ✅ **Modern foundation** - Bzlmod structure ready for future migration expansion as patches become unnecessary
- ✅ **Hybrid compatibility** - Works seamlessly with both bzlmod and traditional WORKSPACE modes

This migration establishes modern Bazel dependency management while maintaining production stability through careful preservation of all custom patches and automatic mode detection.

## Implementation Details

### Automatic Bzlmod Detection

The migration utilizes automatic dependency detection to prevent loading dependencies twice when bzlmod is enabled. This detection works through two mechanisms:

**1. Built-in Detection in `external_http_archive`**

Most Envoy dependencies use the `external_http_archive()` function, which automatically includes a check to prevent double-loading:

```python
# In envoy_http_archive (called by external_http_archive):
def envoy_http_archive(name, locations, **kwargs):
    if name not in native.existing_rules():
        # Only create the repository if it doesn't already exist
        http_archive(name=name, ...)
```

This means that when MODULE.bazel loads a dependency like `google_benchmark`, the subsequent call to `external_http_archive("com_github_google_benchmark")` in repositories.bzl will automatically detect that the dependency is already loaded and skip it.

**2. Manual Detection for Special Cases**

Some dependencies that don't use `external_http_archive()` require explicit checks:

```python
# For googleapis imports that use switched_rules_by_language
if "com_google_googleapis_imports" not in native.existing_rules():
    switched_rules_by_language(
        name = "com_google_googleapis_imports",
        cc = True,
        go = True, 
        python = True,
        grpc = True,
    )
```

### Why This Approach Works

This automatic detection approach provides several benefits:

- **No manual parameters needed**: Unlike approaches that require `bzlmod=True` flags, the system automatically detects the current mode
- **Fail-safe behavior**: If a dependency isn't in MODULE.bazel, it falls back to WORKSPACE.bzlmod loading  
- **Minimal code changes**: Most dependency loading code remains unchanged, only the MODULE.bazel file needs to be added
- **Easy debugging**: You can easily see which dependencies are loaded from which source

### Local Module Support
```python
bazel_dep(name = "envoy_api", version = "0.0.0-dev")
bazel_dep(name = "envoy_build_config", version = "0.0.0-dev")

local_path_override(module_name = "envoy_api", path = "api")
local_path_override(module_name = "envoy_build_config", path = "mobile/envoy_build_config")
```

### Mobile Module Configuration

The mobile directory has its own MODULE.bazel file that extends the root configuration:

```python
# mobile/MODULE.bazel
module(name = "envoy_mobile", version = "0.0.0-dev")

# Additional dependencies specific to mobile builds
bazel_dep(name = "envoy", version = "0.0.0-dev")
bazel_dep(name = "envoy_api", version = "0.0.0-dev")
bazel_dep(name = "envoy_build_config", version = "0.0.0-dev")
bazel_dep(name = "gazelle", version = "0.45.0")
bazel_dep(name = "googleapis", version = "0.0.0-20250703-f9d6fe4a")
bazel_dep(name = "rules_pkg", version = "1.1.0")
bazel_dep(name = "rules_python", version = "1.4.1")

# Local overrides pointing to parent directories
local_path_override(module_name = "envoy", path = "..")
local_path_override(module_name = "envoy_api", path = "../api")
local_path_override(module_name = "envoy_build_config", path = "envoy_build_config")
```

## Future Expansion

Dependencies can be migrated to MODULE.bazel when:
1. BCR versions include equivalent functionality to current patches
2. Patches are no longer needed due to upstream fixes
3. Custom Envoy requirements are met by standard BCR releases

This ensures production stability while enabling modern Bazel dependency management.
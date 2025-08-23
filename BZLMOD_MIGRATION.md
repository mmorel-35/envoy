# Envoy Bzlmod Migration

This document describes the bzlmod migration implemented for Envoy to prepare for Bazel 8.0, which will drop WORKSPACE support, while preserving all existing functionality through dedicated module extensions.

## Overview

This migration implements a comprehensive approach to prepare for Bazel 8.0's removal of WORKSPACE support. It migrates clean dependencies (without patches) to MODULE.bazel and creates dedicated module extensions for the three main Envoy dependency functions, enabling both main module and submodule usage.

## Problem Being Solved

Bazel 8.0 will drop support for the traditional WORKSPACE system, requiring migration to the MODULE.bazel (bzlmod) system. This migration must preserve all existing custom patches that Envoy requires and make the dependency management functions available to all submodules.

## Solution: Dedicated Extensions for Envoy Dependencies

### Key Strategy
- **Clean dependencies** (no patches) migrate to MODULE.bazel as `bazel_dep`
- **Main Envoy functions** are wrapped in dedicated bzlmod extensions:
  - `envoy_dependencies_ext` - Core dependency definitions
  - `envoy_dependencies_extra_ext` - Second-stage dependencies  
  - `envoy_dependency_imports_ext` - Toolchain imports and registrations
- **Zero code duplication** - Extensions directly call existing functions
- **Submodule support** - All submodules can use the same extensions
- **Context-aware behavior** - Automatic bzlmod vs WORKSPACE detection

### Migration Architecture

#### 1. Individual Function Extensions

Instead of a monolithic extension, each major Envoy function gets its own extension:

```starlark
# bazel/repositories.bzl
def _envoy_dependencies_impl(module_ctx):
    envoy_dependencies()

envoy_dependencies_ext = module_extension(
    implementation = _envoy_dependencies_impl,
    doc = "Extension for Envoy's main dependencies."
)

# bazel/repositories_extra.bzl  
def _envoy_dependencies_extra_impl(module_ctx):
    envoy_dependencies_extra()

envoy_dependencies_extra_ext = module_extension(
    implementation = _envoy_dependencies_extra_impl,
    doc = "Extension for Envoy's extra dependencies."
)

# bazel/dependency_imports.bzl
def _envoy_dependency_imports_impl(module_ctx):
    envoy_dependency_imports()

envoy_dependency_imports_ext = module_extension(
    implementation = _envoy_dependency_imports_impl,
    doc = "Extension for Envoy's dependency imports."
)
```

#### 2. Context-Aware Behavior

Extensions use bzlmod context detection to exclude problematic operations:

```starlark
# Bzlmod context detection
_IS_BZLMOD = str(Label("//:invalid")).startswith("@@")

# Exclude native.bind() calls in bzlmod context
if not _IS_BZLMOD:
    native.bind(name = "ssl", actual = "@envoy//bazel:boringssl")
```

### Usage Patterns

#### Main Module (MODULE.bazel)
```starlark
# Envoy dependencies extensions - migrated from WORKSPACE rules
envoy_deps = use_extension("//bazel:repositories.bzl", "envoy_dependencies_ext")
envoy_deps_extra = use_extension("//bazel:repositories_extra.bzl", "envoy_dependencies_extra_ext")  
envoy_imports = use_extension("//bazel:dependency_imports.bzl", "envoy_dependency_imports_ext")
```

#### Submodules (mobile/MODULE.bazel)
```starlark
# Envoy dependencies extensions - inherited from parent envoy module
envoy_deps = use_extension("@envoy//bazel:repositories.bzl", "envoy_dependencies_ext")
envoy_deps_extra = use_extension("@envoy//bazel:repositories_extra.bzl", "envoy_dependencies_extra_ext")  
envoy_imports = use_extension("@envoy//bazel:dependency_imports.bzl", "envoy_dependency_imports_ext")
```

#### API Module (api/MODULE.bazel)
```starlark
# API dependencies extension - migrated from WORKSPACE rule
api_deps = use_extension("//bazel:repositories.bzl", "api_dependencies_ext")
```

### WORKSPACE.bzlmod Migration

WORKSPACE.bzlmod files are updated to remove function calls now handled by extensions:

**Before:**
```starlark
load("@envoy//bazel:repositories.bzl", "envoy_dependencies")
envoy_dependencies()

load("@envoy//bazel:repositories_extra.bzl", "envoy_dependencies_extra")
envoy_dependencies_extra(ignore_root_user_error=True)

load("@envoy//bazel:dependency_imports.bzl", "envoy_dependency_imports")
envoy_dependency_imports()
```

**After:**
```starlark
# envoy_dependencies(), envoy_dependencies_extra(), and envoy_dependency_imports()
# are now handled by bzlmod extensions declared in MODULE.bazel:
# - envoy_dependencies_ext from "//bazel:repositories.bzl" 
# - envoy_dependencies_extra_ext from "//bazel:repositories_extra.bzl"
# - envoy_dependency_imports_ext from "//bazel:dependency_imports.bzl"
```

## Benefits

- ✅ **Bazel 8.0 ready** with proper bzlmod extension architecture
- ✅ **Preserves all functionality** through direct function calls
- ✅ **No code duplication** - extensions wrap existing functions
- ✅ **Context-aware** - handles bzlmod vs WORKSPACE differences automatically
- ✅ **Submodule support** - all submodules can use the same extensions
- ✅ **Maintainable structure** - extensions defined where functions are implemented
- ✅ **Follows naming conventions** - uses `_ext` suffix as recommended

## Submodule Integration

All Envoy submodules can now use the same extension pattern:

### Mobile Module (mobile/MODULE.bazel)
```starlark
module(name = "envoy_mobile", version = "0.0.0-dev")

bazel_dep(name = "envoy", version = "0.0.0-dev")
bazel_dep(name = "envoy_api", version = "0.0.0-dev")
# ... other dependencies

# Envoy dependencies extensions - inherited from parent envoy module
envoy_deps = use_extension("@envoy//bazel:repositories.bzl", "envoy_dependencies_ext")
envoy_deps_extra = use_extension("@envoy//bazel:repositories_extra.bzl", "envoy_dependencies_extra_ext")  
envoy_imports = use_extension("@envoy//bazel:dependency_imports.bzl", "envoy_dependency_imports_ext")
```

### API Module (api/MODULE.bazel)  
```starlark
module(name = "envoy_api", version = "0.0.0-dev")

# ... api-specific dependencies

# API dependencies extension
api_deps = use_extension("//bazel:repositories.bzl", "api_dependencies_ext")
```

### Build Config Module (mobile/envoy_build_config/MODULE.bazel)
```starlark
module(name = "envoy_build_config", version = "0.0.0-dev")

bazel_dep(name = "envoy", version = "0.0.0-dev")
# Simple module that inherits dependencies from parent
```

## Implementation Details

### Extension Definitions

**1. Main Dependencies (bazel/repositories.bzl)**
```starlark
def _envoy_dependencies_impl(module_ctx):
    """Implementation of the envoy_dependencies extension."""
    envoy_dependencies()

envoy_dependencies_ext = module_extension(
    implementation = _envoy_dependencies_impl,
    doc = """
    Extension for Envoy's main dependencies.
    
    This extension wraps the envoy_dependencies() function to make it
    available as a bzlmod module extension, following the conservative 
    bzlmod migration strategy.
    """,
)
```

**2. Extra Dependencies (bazel/repositories_extra.bzl)**
```starlark  
def _envoy_dependencies_extra_impl(module_ctx):
    """Implementation of the envoy_dependencies_extra extension."""
    envoy_dependencies_extra()

envoy_dependencies_extra_ext = module_extension(
    implementation = _envoy_dependencies_extra_impl,
    doc = """
    Extension for Envoy's extra dependencies.
    
    This extension wraps the envoy_dependencies_extra() function to make it
    available as a bzlmod module extension. These are dependencies that rely  
    on a first stage of dependency loading in envoy_dependencies().
    """,
)
```

**3. Dependency Imports (bazel/dependency_imports.bzl)**
```starlark
def _envoy_dependency_imports_impl(module_ctx):
    """Implementation of the envoy_dependency_imports extension.""" 
    envoy_dependency_imports()

envoy_dependency_imports_ext = module_extension(
    implementation = _envoy_dependency_imports_impl,
    doc = """
    Extension for Envoy's dependency imports.
    
    This extension wraps the envoy_dependency_imports() function to make it
    available as a bzlmod module extension. This handles toolchain imports
    and registrations that depend on repositories loaded in earlier stages.
    """,
)
```

### Automatic Conflict Detection

The extensions leverage existing automatic dependency detection mechanisms:

**1. Built-in Detection in `external_http_archive`**
```starlark
# Most Envoy dependencies automatically check for existing rules
def external_http_archive(name, **kwargs):
    envoy_http_archive(name, locations = REPOSITORY_LOCATIONS, **kwargs)

def envoy_http_archive(name, locations, **kwargs):
    if name not in native.existing_rules():
        http_archive(name=name, ...)  # Only create if doesn't exist
```

**2. Context-Aware Bind Operations**
```starlark
# Bzlmod context detection
_IS_BZLMOD = str(Label("//:invalid")).startswith("@@")

# Skip problematic operations in bzlmod context
if not _IS_BZLMOD:
    native.bind(name = "ssl", actual = "@envoy//bazel:boringssl")
```

## Dependencies Overview

### Migrated to MODULE.bazel (Clean Dependencies)
These dependencies have no patches and are available in the Bazel Central Registry:

- **bazel_features** (1.33.0) - Essential platform detection
- **platforms** (1.0.0) - Platform configuration
- **google_benchmark** (1.9.4) - Performance benchmarking
- **googleapis** (0.0.0-20241220-5e258e33.bcr.1) - Google APIs with switched_rules extension
- **spdlog** (1.15.3) - Logging library  
- **fmt** (11.2.0) - String formatting
- **gazelle** (0.45.0) - Bazel BUILD file generator for Go projects
- **xxhash** (0.8.3) - Hash function library
- **protoc-gen-validate** (1.2.1.bcr.1) - Protocol buffer validation
- **rules_go** (0.53.0) - Bazel rules for the Go language
- **rules_license** (1.0.0) - Bazel rules for checking open source licenses
- **rules_python** (0.35.0) - Python build rules
- **rules_cc** (0.1.4) - C++ build rules
- **rules_pkg** (1.1.0) - Package creation rules
- **rules_shell** (0.5.1) - Shell script build rules
- **rules_shellcheck** (0.3.3) - Shell script linting

### Managed by Extensions
All dependencies with patches, custom configurations, or complex build requirements are now managed through the dedicated extensions:

#### envoy_dependencies_ext
Core dependencies including:
- **protobuf** - Extensive patches for arena.h modifications
- **abseil-cpp** - Custom compatibility patches
- **grpc** - Custom modifications
- **googletest** - Custom patches
- **boringssl** - Standard and FIPS variants with patches
- All HTTP archives defined in `bazel/repositories.bzl`

#### envoy_dependencies_extra_ext  
Second-stage dependencies including:
- **rules_foreign_cc** - Foreign C/C++ build rules
- **rules_rust** - Rust build rules with platform-specific patches
- **proxy_wasm_cpp_host** - WebAssembly support
- Python toolchain registrations
- Aspect Bazel lib dependencies

#### envoy_dependency_imports_ext
Toolchain imports and registrations including:
- Go toolchain registration and SDK downloads
- Apple rules dependencies
- Pip dependency installations (base, dev, fuzzing)
- Emscripten toolchain registration
- Protobuf gRPC toolchain registrations
- Foreign CC dependencies setup

### API Module Dependencies (api_dependencies_ext)
The API module has its own extension for API-specific dependencies:
- **bazel_skylib**
- **rules_jvm_external**  
- **com_envoyproxy_protoc_gen_validate** (with pgv.patch)
- **com_google_googleapis**
- **com_github_cncf_xds**
- **prometheus_metrics_model**
- **rules_buf**, **rules_proto**
- **com_github_openzipkin_zipkinapi**
- **opentelemetry_proto**
- **dev_cel**
- **com_github_chrusty_protoc_gen_jsonschema**

## Migration Benefits

### For Development
- ✅ **Consistent patterns** - All modules use the same extension approach
- ✅ **Clear separation** - Clean vs patched dependencies clearly differentiated
- ✅ **Easy maintenance** - Extensions defined alongside their functions
- ✅ **Reduced duplication** - Submodules inherit from parent extensions

### For Operations  
- ✅ **Bazel 8.0 ready** - Full bzlmod foundation established
- ✅ **Zero functionality loss** - All patches and configurations preserved
- ✅ **Automatic conflict prevention** - Built-in dependency detection
- ✅ **Incremental migration path** - Individual dependencies can be moved to MODULE.bazel over time

### For Contributors
- ✅ **Familiar patterns** - Extension names match function names
- ✅ **Documentation alignment** - Extensions defined where functions are implemented
- ✅ **Submodule consistency** - Same extension usage across all modules
- ✅ **Clear migration examples** - Both main and submodule patterns documented

## Future Migration Path

Dependencies can be gradually moved from extensions to MODULE.bazel when:

1. **Upstream patches are accepted** and included in BCR releases
2. **Custom patches are no longer needed** due to compatibility improvements
3. **Complex configurations are simplified** or become standard
4. **BCR versions meet Envoy requirements** without modifications

**Migration Process:**
1. Verify dependency works without patches in BCR
2. Add as `bazel_dep()` in MODULE.bazel
3. Remove from appropriate extension implementation
4. Test all affected modules
5. Update documentation

This migration establishes a modern, maintainable Bazel dependency management system while preserving all existing functionality and providing clear patterns for both main and submodule usage.
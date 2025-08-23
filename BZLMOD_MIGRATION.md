# Envoy Bzlmod Migration

This document describes the conservative bzlmod migration implemented for Envoy to prepare for Bazel 8.0, which will drop WORKSPACE support, while preserving all custom patches and using automatic detection.

## Overview

This migration implements a conservative approach to prepare for Bazel 8.0's removal of WORKSPACE support. It migrates clean dependencies (without patches) to MODULE.bazel and uses a custom `non_module_dependencies` extension for all dependencies requiring custom patches or complex configurations.

## Problem Being Solved

Bazel 8.0 will drop support for the traditional WORKSPACE system, requiring migration to the MODULE.bazel (bzlmod) system. This migration must preserve all existing custom patches that Envoy requires for proper functionality.

## Solution: Conservative Patch-Preserving Migration with Extension

### Key Strategy
- **Clean dependencies** (no patches) migrate to MODULE.bazel as `bazel_dep`
- **Patched dependencies** use the `non_module_dependencies` extension to preserve modifications
- **Automatic bzlmod detection** using built-in `native.existing_rules()` checks
- **Hybrid approach** ensures both bzlmod benefits and patch preservation

### Migration Criteria

**Dependencies migrated to MODULE.bazel:**
- No custom patches required
- Available in Bazel Central Registry (BCR) with compatible versions
- Standard build configuration (no special flags or build files)
- No complex repository rules or transformations needed

**Dependencies in non_module_dependencies extension:**
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
bazel_dep(name = "gazelle", version = "0.45.0")
bazel_dep(name = "google_benchmark", version = "1.9.4")
bazel_dep(name = "googleapis", version = "0.0.0-20241220-5e258e33.bcr.1")
bazel_dep(name = "platforms", version = "1.0.0") 
bazel_dep(name = "protoc-gen-validate", version = "1.2.1.bcr.1")
bazel_dep(name = "rules_python", version = "0.35.0")
bazel_dep(name = "rules_cc", version = "0.1.4")
bazel_dep(name = "rules_go", version = "0.53.0")
bazel_dep(name = "rules_license", version = "1.0.0")
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

# Non-module dependencies extension for patched dependencies
non_module_deps = use_extension("//bazel:repositories.bzl", "non_module_dependencies_rule")
use_repo(non_module_deps, "com_google_absl", "com_google_protobuf", "boringssl", ...)
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

### Non-Module Dependencies Extension

Dependencies requiring patches or custom configurations use the `non_module_dependencies_rule` extension defined in `//bazel:repositories.bzl`:

```python
non_module_deps = use_extension("//bazel:repositories.bzl", "non_module_dependencies_rule")
use_repo(
    non_module_deps,
    "com_google_absl",        # Custom abseil.patch
    "com_google_protobuf",    # Custom protobuf.patch  
    "com_google_googletest",  # Custom googletest.patch
    "com_github_grpc_grpc",   # Custom grpc.patch
    "boringssl",              # BoringSSL with FIPS patches
    "boringssl_fips",         # FIPS-specific variant
    # ... all other patched dependencies
)
```

This extension is defined in the same file (`bazel/repositories.bzl`) where the dependencies are actually used, following the suggested `_rule` suffix pattern. The extension automatically calls all the private dependency functions that handle patching and custom build configurations.

**2. WORKSPACE.bzlmod - Fallback for Compatibility**
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

#### Managed by non_module_dependencies Extension
All dependencies not migrated to MODULE.bazel are now managed through the `non_module_dependencies_rule` extension defined in `bazel/repositories.bzl`. This includes:

**Core Dependencies with Patches:**
- **protobuf** - Extensive `protobuf.patch` for arena.h modifications
- **abseil-cpp** - Custom `abseil.patch` for compatibility
- **grpc** - Custom `grpc.patch` modifications
- **googletest** - Custom `googletest.patch`

**Security Dependencies:**
- **boringssl** - Standard BoringSSL
- **boringssl_fips** - FIPS support patches (`boringssl_fips.patch`)
- **aws_lc** - Alternative cryptographic library

**Build Tool Dependencies:**
- **rules_foreign_cc** - Foreign C/C++ build rules with patches
- **rules_rust** - Rust build rules with platform-specific patches
- **bazel_toolchains** - Bazel toolchain configurations
- **rules_fuzzing** - Fuzzing test support

**Language and Protocol Dependencies:**
- **v8** - JavaScript engine with extensive patches
- **com_google_cel_cpp** - Common Expression Language for C++
- **com_github_grpc_grpc** - gRPC with custom patches
- **com_google_quiche** - QUIC/HTTP/3 implementation

**Networking and I/O Dependencies:**
- **c-ares** - DNS resolution with custom patches
- **nghttp2** - HTTP/2 implementation
- **libevent** - Event notification library
- **io_opentelemetry_cpp** - OpenTelemetry C++ client

**Compression and Encoding:**
- **org_brotli** - Brotli compression
- **com_github_facebook_zstd** - Zstandard compression
- **com_github_lz4_lz4** - LZ4 compression
- **net_zlib** - zlib compression library

**Performance and Monitoring:**
- **com_github_gperftools_gperftools** - Google performance tools
- **com_github_google_tcmalloc** - High-performance memory allocator
- **intel_ittapi** - Intel Instrumentation and Tracing Technology API

**Platform-Specific Dependencies:**
- **com_github_fdio_vpp_vcl** - Vector Packet Processing
- **com_github_axboe_liburing** - Linux io_uring library
- **intel_dlb** - Intel Dynamic Load Balancer
- **com_github_intel_qatlib** - Intel QuickAssist Technology

**WebAssembly Support:**
- **proxy_wasm_cpp_sdk** - Proxy-Wasm C++ SDK
- **proxy_wasm_cpp_host** - Proxy-Wasm host implementation
- **proxy_wasm_rust_sdk** - Proxy-Wasm Rust SDK
- **com_github_wamr** - WebAssembly Micro Runtime
- **com_github_wasmtime** - WebAssembly runtime

**Additional Libraries:**
- **com_github_jbeder_yaml_cpp** - YAML parser and emitter
- **com_github_nlohmann_json** - JSON parser
- **com_github_skyapm_cpp2sky** - SkyWalking C++ agent
- **com_github_maxmind_libmaxminddb** - MaxMind DB reader

All these dependencies are automatically loaded through the extension, which handles their patches, custom build files, and complex configurations while maintaining compatibility with bzlmod.

## Impact

- ✅ **Prepares for Bazel 8.0** - Establishes bzlmod foundation before WORKSPACE deprecation in Bazel 8.0
- ✅ **Preserves all patches** - No functionality regression, all custom modifications maintained through extension
- ✅ **Conservative approach** - Minimal risk by only migrating dependencies without patches to MODULE.bazel
- ✅ **Automatic detection** - Built-in bzlmod mode detection without requiring manual configuration
- ✅ **Modern foundation** - Bzlmod structure ready for future migration expansion as patches become unnecessary
- ✅ **Hybrid compatibility** - Works seamlessly with both bzlmod and traditional WORKSPACE modes
- ✅ **Centralized management** - All non-migrated dependencies managed through single extension
- ✅ **Documentation sync** - Extension defined in same file where dependencies are used (`bazel/repositories.bzl`)

This migration establishes modern Bazel dependency management while maintaining production stability through careful preservation of all custom patches via the `non_module_dependencies` extension.

## Implementation Details

### Non-Module Dependencies Extension

The `non_module_dependencies_rule` extension is defined in `bazel/repositories.bzl` (following the suggested `_rule` suffix pattern) and provides a centralized way to manage all dependencies that cannot yet be migrated to MODULE.bazel due to patches or complex configurations.

**Extension Definition:**
```python
# In bazel/repositories.bzl
def _non_module_dependencies_impl(module_ctx):
    """
    Implementation of the non_module_dependencies extension.
    
    This extension defines all dependencies that are not yet migrated to 
    MODULE.bazel due to patches, custom build files, or complex configurations.
    """
    # Core dependencies with patches
    _com_google_absl()  # Custom abseil.patch
    _com_google_googletest()  # Custom googletest.patch  
    _com_google_protobuf()  # Custom protobuf.patch
    _com_github_grpc_grpc()  # Custom grpc.patch
    
    # BoringSSL with FIPS support
    _boringssl()
    _boringssl_fips()
    _aws_lc()
    
    # All other dependencies with patches or custom configurations
    # ... (full list in implementation)

non_module_dependencies_rule = module_extension(
    implementation = _non_module_dependencies_impl,
    doc = "Extension for Envoy dependencies not yet migrated to MODULE.bazel"
)
```

**Usage in MODULE.bazel:**
```python
non_module_deps = use_extension("//bazel:repositories.bzl", "non_module_dependencies_rule")
use_repo(
    non_module_deps,
    "com_google_absl",
    "com_google_protobuf", 
    "boringssl",
    # ... all other non-migrated dependencies
)
```

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

Dependencies can be gradually migrated from the `non_module_dependencies` extension to MODULE.bazel when:

1. **BCR versions include equivalent functionality** to current patches
2. **Patches are no longer needed** due to upstream fixes
3. **Custom Envoy requirements** are met by standard BCR releases
4. **Complex build configurations** are simplified or standardized

**Migration Process:**
1. Remove dependency from `non_module_dependencies_rule` extension
2. Add as `bazel_dep()` in MODULE.bazel
3. Remove from `use_repo()` list
4. Update documentation

**Benefits of Extension Approach:**
- ✅ **Centralized management** - All non-migrated dependencies in one place
- ✅ **Easy tracking** - Clear separation between migrated and non-migrated dependencies
- ✅ **Documentation sync** - Extension defined where dependencies are used
- ✅ **Future-proof** - Easy to migrate individual dependencies over time
- ✅ **Automatic conflict prevention** - Built-in bzlmod detection
- ✅ **Conservative safety** - Preserves all existing functionality

This ensures production stability while enabling modern Bazel dependency management and providing a clear path for future migration expansion.
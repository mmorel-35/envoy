# Envoy Bzlmod Migration

This document describes the bzlmod migration implemented for Envoy to prepare for Bazel 8.0, which will drop WORKSPACE support, while preserving all existing functionality through dedicated module extensions.

## Overview

This migration implements a comprehensive approach to prepare for Bazel 8.0's removal of WORKSPACE support. It includes:

1. Migration of clean dependencies to MODULE.bazel
2. Dedicated module extensions for Envoy dependency functions  
3. **Compatibility layer for //external: to //third_party: migration**
4. Support for both main module and all submodules

## Dependencies and External References Migration

### Legacy //external: Compatibility Layer

A `third_party/BUILD.bazel` compatibility layer provides aliases for all legacy `//external:foo` references:

```starlark
alias(name = "ssl", actual = "@envoy//bazel:boringssl")
alias(name = "protobuf", actual = "@com_google_protobuf//:protobuf")
```

The `envoy_external_dep_path()` function now redirects to `//third_party:` instead of `//external:`. 

**For new code**: Use direct `@repo//:target` dependencies instead of `//external:` or `//third_party:` references.

See `THIRD_PARTY_MIGRATION.md` for detailed migration strategy.

## Problem Being Solved

Bazel 8.0 will drop support for the traditional WORKSPACE system, requiring migration to the MODULE.bazel (bzlmod) system. This migration must preserve all existing custom patches that Envoy requires and make the dependency management functions available to all submodules.

## Solution: Dedicated Extensions + Upstream Rules Migration

### Key Strategy
- **Clean dependencies** (no patches) migrate to MODULE.bazel as `bazel_dep`
- **Main Envoy functions** are wrapped in dedicated bzlmod extensions with consistent naming:
  - `envoy_dependencies_ext` - Core dependency definitions
  - `envoy_dependencies_extra_ext` - Second-stage dependencies  
  - `envoy_dependency_imports_ext` - Toolchain imports and registrations
  - `envoy_api_dependencies_ext` - API-specific dependencies
  - `envoy_mobile_dependencies_ext` - Mobile-specific dependencies (Swift, Kotlin, etc.)
- **Upstream rules_python migration** - Python dependencies use standard `@rules_python//python/extensions:pip.bzl`
- **Zero code duplication** - Extensions directly call existing functions
- **Submodule support** - All submodules can use the same extensions
- **Context-aware behavior** - Automatic bzlmod vs WORKSPACE detection

### Migration to Upstream Extensions

Where possible, Envoy now uses **upstream bzlmod extensions** instead of custom wrappers:

**Python Dependencies (✅ Migrated)**:
```starlark
# Instead of custom envoy_python_dependencies_ext, use upstream:
python = use_extension("@rules_python//python/extensions:python.bzl", "python")
pip = use_extension("@rules_python//python/extensions:pip.bzl", "pip")
```

**Benefits of upstream migration**:
- Standardized approach maintained by rules authors
- Better long-term maintenance and feature support
- Consistent patterns across the Bazel ecosystem
- Reduced custom extension maintenance burden

**Custom extensions preserved for**:
- Complex repository setup (`envoy_api_binding_ext`, `envoy_repo_ext`)
- Envoy-specific dependency patching and configuration
- Mobile platform-specific tooling (Swift, Kotlin, Android)

### Migration Architecture

**Complete Bzlmod Extension Coverage:**
All dependency functions now have corresponding bzlmod extensions or use upstream alternatives:

**Custom Envoy Extensions (for complex/patched dependencies):**
- `envoy_dependencies_ext` - Core dependency definitions
- `envoy_dependencies_extra_ext` - Second-stage dependencies  
- `envoy_dependency_imports_ext` - Toolchain imports and registrations
- `envoy_dependency_imports_extra_ext` - Additional dependency imports
- `envoy_api_dependencies_main_ext` - Main API dependencies wrapper
- `envoy_api_dependencies_ext` - API submodule dependencies
- `envoy_mobile_dependencies_ext` - Mobile-specific dependencies
- `envoy_api_binding_ext` - API repository binding setup
- `envoy_repo_ext` - Repository metadata setup

**Upstream Extensions (for standardized dependencies):**
- Python dependencies: `@rules_python//python/extensions:pip.bzl` (replaces `envoy_python_dependencies_ext`)
- Python toolchains: `@rules_python//python/extensions:python.bzl`

**WORKSPACE.bzlmod Elimination:**
All repository setup and custom logic previously in WORKSPACE.bzlmod has been migrated to dedicated module extensions. WORKSPACE.bzlmod has been completely removed, achieving full bzlmod compatibility.

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

# api/bazel/repositories.bzl
def _api_dependencies_impl(module_ctx):
    api_dependencies()

envoy_api_dependencies_ext = module_extension(
    implementation = _api_dependencies_impl,
    doc = "Extension for Envoy API's dependencies."
)

# mobile/bazel/envoy_mobile_dependencies.bzl
def _envoy_mobile_dependencies_impl(module_ctx):
    envoy_mobile_dependencies()

envoy_mobile_dependencies_ext = module_extension(
    implementation = _envoy_mobile_dependencies_impl,
    doc = "Extension for Envoy Mobile's dependencies."
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
# Core Envoy extensions (custom)
envoy_deps = use_extension("//bazel:repositories.bzl", "envoy_dependencies_ext")
envoy_deps_extra = use_extension("//bazel:repositories_extra.bzl", "envoy_dependencies_extra_ext")
envoy_imports = use_extension("//bazel:dependency_imports.bzl", "envoy_dependency_imports_ext")

# Python dependencies (upstream rules_python - preferred approach)
python = use_extension("@rules_python//python/extensions:python.bzl", "python")
python.toolchain(python_version = "3.12")

pip = use_extension("@rules_python//python/extensions:pip.bzl", "pip")
pip.parse(hub_name = "base_pip3", python_version = "3.12", requirements_lock = "//tools/base:requirements.txt")
pip.parse(hub_name = "dev_pip3", python_version = "3.12", requirements_lock = "//tools/dev:requirements.txt")
use_repo(pip, "base_pip3", "dev_pip3", "fuzzing_pip3")

# Repository setup (custom extensions for complex logic)
envoy_api_binding = use_extension("//bazel:api_binding.bzl", "envoy_api_binding_ext")
envoy_repo_setup = use_extension("//bazel:repo.bzl", "envoy_repo_ext")
```

#### Submodules (mobile/MODULE.bazel)
```starlark
# Envoy dependencies extensions - inherited from parent envoy module
envoy_deps = use_extension("@envoy//bazel:repositories.bzl", "envoy_dependencies_ext")
envoy_deps_extra = use_extension("@envoy//bazel:repositories_extra.bzl", "envoy_dependencies_extra_ext")  
envoy_imports = use_extension("@envoy//bazel:dependency_imports.bzl", "envoy_dependency_imports_ext")

# Mobile-specific dependencies extension
envoy_mobile_deps = use_extension("//bazel:envoy_mobile_dependencies.bzl", "envoy_mobile_dependencies_ext")

# Python dependencies (upstream rules_python - same pattern as main)
python = use_extension("@rules_python//python/extensions:python.bzl", "python", dev_dependency = True)
python.toolchain(python_version = "3.12")

pip = use_extension("@rules_python//python/extensions:pip.bzl", "pip", dev_dependency = True)
pip.parse(hub_name = "base_pip3", python_version = "3.12", requirements_lock = "@envoy//tools/base:requirements.txt")
use_repo(pip, "base_pip3")
```

#### API Module (api/MODULE.bazel)
```starlark
# API dependencies extension - migrated from WORKSPACE rule with consistent naming
envoy_api_deps = use_extension("//bazel:repositories.bzl", "envoy_api_dependencies_ext")
# envoy_api_dependencies_ext handles API-specific dependencies and external archives
```

### WORKSPACE.bzlmod Migration  

**Completed**: WORKSPACE.bzlmod has been completely eliminated. All repository setup and custom logic previously in WORKSPACE.bzlmod has been migrated to dedicated module extensions, achieving full bzlmod compatibility ready for Bazel 8.0.

### Upstream Extensions Migration Benefits

**Repository Maintenance Impact:**
- **Reduced maintenance burden**: Upstream extensions are maintained by rules authors, reducing Envoy-specific maintenance 
- **Better feature support**: Upstream extensions get new features and bug fixes automatically
- **Ecosystem alignment**: Consistent patterns across Bazel ecosystem improve developer experience
- **Future-proofing**: Upstream extensions evolve with Bazel releases and best practices

**Migration Strategy:**
- ✅ **Python dependencies**: Migrated to `@rules_python//python/extensions:pip.bzl` 
- 🔄 **Potential future migrations**:
  - **Go dependencies**: Could migrate to `@rules_go//go:extensions.bzl` for standard Go modules
  - **Proto dependencies**: Could use `@rules_proto//proto:extensions.bzl` for standard proto compilation
  - **Java dependencies**: Could use `@rules_jvm_external//private/rules:extensions.bzl` for Maven dependencies
  - **Container images**: Could use `@rules_oci//oci:extensions.bzl` for container build rules
- 🔒 **Custom extensions preserved**: Complex Envoy-specific logic (API binding, repository metadata, patched dependencies) remains custom

**Evaluation Criteria for Future Migrations**:
- Extension provides equivalent functionality to custom implementation
- Upstream extension supports required configuration (patches, custom args)
- Migration doesn't break existing Envoy-specific workflows
- Clear maintenance and support benefits outweigh migration costs

**Performance and Reliability:**
- Upstream extensions often have better caching and performance optimizations
- Standardized error handling and debugging capabilities  
- Better integration with Bazel's dependency resolution algorithms

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
# envoy_dependencies(), envoy_dependencies_extra(), envoy_dependency_imports(),
# and envoy_mobile_dependencies() are now handled by bzlmod extensions declared in MODULE.bazel:
# - envoy_dependencies_ext from "//bazel:repositories.bzl" 
# - envoy_dependencies_extra_ext from "//bazel:repositories_extra.bzl"
# - envoy_dependency_imports_ext from "//bazel:dependency_imports.bzl"
# - envoy_mobile_dependencies_ext from "//bazel:envoy_mobile_dependencies.bzl"
```

## use_repo() Usage and Rationale

### Migration Clarity and Maintainability

While many Envoy extensions manage repositories internally and don't expose specific dependencies through `use_repo()`, this mechanism is valuable for:

1. **Migration Transparency**: Explicitly declaring which extensions provide which repositories helps during WORKSPACE to bzlmod migration
2. **Future Maintenance**: As dependencies move from extensions to direct `bazel_dep()` declarations, `use_repo()` provides clear documentation of what needs to be migrated
3. **Debugging**: When dependency resolution fails, `use_repo()` statements help identify which extensions should provide missing repositories
4. **Consistency**: Following bzlmod best practices improves code readability and maintainability

### Extension-Specific Patterns

**Extensions without use_repo (Internal Repository Management):**
```starlark
# Extensions that manage all repositories internally
envoy_deps = use_extension("//bazel:repositories.bzl", "envoy_dependencies_ext")
# All repositories (protobuf, grpc, boringssl, etc.) are available implicitly

envoy_deps_extra = use_extension("//bazel:repositories_extra.bzl", "envoy_dependencies_extra_ext")  
# Toolchain registrations and complex dependencies handled internally
```

**Extensions with use_repo (Explicit Repository Exposure):**
```starlark
# Extensions that expose specific repositories for external use
switched_rules = use_extension("@com_google_googleapis//:extensions.bzl", "switched_rules")
switched_rules.use_languages(cc = True, go = True, grpc = True, python = True)
use_repo(switched_rules, "com_google_googleapis_imports")
# Explicitly expose the imports repository for downstream usage
```

### When to Use use_repo()

According to Bazel documentation, `use_repo()` should be used when:
- Extensions expose repositories that are used directly in BUILD files
- Repositories need to be available to other modules or extensions
- Clear dependency documentation is needed for migration planning
- Extensions provide configurable repository sets

For Envoy's internal extensions that wrap existing functions, repositories are typically made available through the same mechanisms as WORKSPACE, making explicit `use_repo()` declarations optional for most cases.

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

# Mobile-specific dependencies extension
envoy_mobile_deps = use_extension("//bazel:envoy_mobile_dependencies.bzl", "envoy_mobile_dependencies_ext")
# envoy_mobile_dependencies_ext handles mobile-specific dependencies like Swift, Kotlin, etc.
```

### API Module (api/MODULE.bazel)  
```starlark
module(name = "envoy_api", version = "0.0.0-dev")

# ... api-specific dependencies

# API dependencies extension with consistent naming
envoy_api_deps = use_extension("//bazel:repositories.bzl", "envoy_api_dependencies_ext")
# envoy_api_dependencies_ext handles API-specific dependencies and external archives
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

### API Module Dependencies (envoy_api_dependencies_ext)
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

### Mobile Module Dependencies (envoy_mobile_dependencies_ext)
The Mobile module has its own extension for mobile-specific dependencies:
- **Swift dependencies** - Apple support, rules, Swift rules
- **Kotlin dependencies** - Java rules, Maven artifacts, Kotlin repositories
- **Android dependencies** - Test artifacts, core libraries
- **Protocol buffer dependencies** - Proto rules, gRPC toolchains
- **Mobile build tools** - Detekt rules, Robolectric repositories

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

## Migration Status: COMPLETE ✅

The bzlmod migration is now complete with Bazel 8.0 readiness achieved:

### ✅ Completed Tasks
- **All dependency functions migrated** - 10 extensions cover all WORKSPACE functions
- **WORKSPACE.bzlmod eliminated** - Complete removal of hybrid workspace dependency  
- **Full extension coverage** - Every repository setup function now has bzlmod extension
- **Consistent naming** - All extensions follow `envoy_*_ext` convention
- **Submodule support** - Mobile, API, and build config modules fully migrated
- **Context-aware behavior** - Automatic bzlmod vs WORKSPACE detection
- **Third-party compatibility** - Legacy //external: references redirected to //third_party:

### Extension Mapping Summary
| Function | Extension | Location |
|----------|-----------|----------|
| `envoy_dependencies()` | `envoy_dependencies_ext` | `bazel/repositories.bzl` |
| `envoy_dependencies_extra()` | `envoy_dependencies_extra_ext` | `bazel/repositories_extra.bzl` |
| `envoy_dependency_imports()` | `envoy_dependency_imports_ext` | `bazel/dependency_imports.bzl` |
| `envoy_dependency_imports_extra()` | `envoy_dependency_imports_extra_ext` | `bazel/dependency_imports_extra.bzl` |
| `envoy_api_dependencies()` | `envoy_api_dependencies_main_ext` | `bazel/api_repositories.bzl` |
| `api_dependencies()` | `envoy_api_dependencies_ext` | `api/bazel/repositories.bzl` |
| `envoy_mobile_dependencies()` | `envoy_mobile_dependencies_ext` | `mobile/bazel/envoy_mobile_dependencies.bzl` |
| `envoy_api_binding()` | `envoy_api_binding_ext` | `bazel/api_binding.bzl` |
| `envoy_repo()` | `envoy_repo_ext` | `bazel/repo.bzl` |
| `envoy_python_dependencies()` | `envoy_python_dependencies_ext` | `bazel/python_dependencies.bzl` |

### Ready for Bazel 8.0
The Envoy repository is now fully prepared for Bazel 8.0's removal of WORKSPACE support. All repository setup, custom logic, and dependency management has been successfully migrated to bzlmod module extensions with zero functionality loss.
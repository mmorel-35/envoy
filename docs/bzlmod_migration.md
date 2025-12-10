# Envoy Bzlmod Migration Documentation

This document tracks the progress, blockers, and recommendations for migrating the Envoy ecosystem to Bazel's bzlmod (Bazel Module) system.

## Overview

The Envoy project is migrating from the traditional WORKSPACE-based dependency management to Bazel's new module system (bzlmod). This migration involves multiple repositories in the Envoy ecosystem:

- **envoy** (main repository) - ✅ This repository
- **envoy_api** (API definitions - local module at `api/`)
- **envoy_mobile** (mobile platform support - local module at `mobile/`)
- **envoy_build_config** (build configuration - local module at `mobile/envoy_build_config/`)
- **envoy_toolshed** (development and CI tooling) - 🔄 Using bzlmod branch via git_override
- **envoy_examples** (example configurations and WASM extensions) - 🔄 Using bzlmod-migration branch via git_override

## Changes Made in This Repository

This repository (envoy) has been updated with the following bzlmod migration changes:

### ✅ Completed Changes

1. **Added bazel_dep declarations for ecosystem modules:**
   - `bazel_dep(name = "envoy_toolshed")` - Runtime dependency
   - `bazel_dep(name = "envoy_examples", dev_dependency = True)` - Dev-only dependency

2. **Added git_override entries to use bzlmod migration branches:**
   - `envoy_examples`: Points to https://github.com/mmorel-35/examples bzlmod-migration branch
   - `envoy_toolshed`: Points to https://github.com/mmorel-35/toolshed bzlmod branch with `strip_prefix = "bazel"`

3. **Updated bazel/repositories.bzl:**
   - Wrapped `envoy_examples` and `envoy_toolshed` http_archive calls with `if not bzlmod:` condition
   - This prevents double-loading when bzlmod is enabled

4. **Removed from envoy_dependencies_extension use_repo:**
   - Removed `envoy_examples` and `envoy_toolshed` from use_repo() call
   - These are now loaded as bazel_dep modules instead of through the extension

5. **Created comprehensive documentation:**
   - This file (docs/bzlmod_migration.md) documents all blockers, recommendations, and migration status

## Migration Status

### Completed Items

✅ **MODULE.bazel created** - Root module file with bazel_dep declarations
✅ **Local path overrides** - envoy_api, envoy_mobile, envoy_build_config use local_path_override
✅ **BCR dependencies migrated** - Core dependencies from Bazel Central Registry are declared as bazel_dep
✅ **Extension system implemented** - Module extensions for non-BCR dependencies in `bazel/extensions.bzl`
✅ **WORKSPACE.bzlmod created** - Empty file to enable bzlmod mode

### Work in Progress

The following git_override entries have been configured to use bzlmod migration branches:

```starlark
# In MODULE.bazel

# XDS protocol definitions (already configured)
git_override(
    module_name = "xds",
    commit = "8bfbf64dc13ee1a570be4fbdcfccbdd8532463f0",
    remote = "https://github.com/cncf/xds",
)

# LLVM toolchain (already configured)
git_override(
    module_name = "toolchains_llvm",
    commit = "fb29f3d53757790dad17b90df0794cea41f1e183",
    remote = "https://github.com/bazel-contrib/toolchains_llvm",
)

# Envoy examples - bzlmod migration branch
# NOTE: See Blocker #1 below - missing version issue in envoy_examples
git_override(
    module_name = "envoy_examples",
    commit = "1ceb95e9c9c8b1892d0c14a1ba4c42216348831d",  # bzlmod-migration branch
    remote = "https://github.com/mmorel-35/examples",
)

# Envoy toolshed - bzlmod migration branch
# Note: strip_prefix points to bazel/ subdirectory where MODULE.bazel is located
git_override(
    module_name = "envoy_toolshed",
    commit = "d718b38e7d0bd7e41394ff48db046b15b20784d5",  # bzlmod branch
    remote = "https://github.com/mmorel-35/toolshed",
    strip_prefix = "bazel",
)
```

**Commit References:**
- envoy_examples: `1ceb95e9c9c8b1892d0c14a1ba4c42216348831d` from bzlmod-migration branch
- envoy_toolshed: `d718b38e7d0bd7e41394ff48db046b15b20784d5` from bzlmod branch

These commits represent the latest state as of the documentation date. To update to newer commits:
```bash
# Get latest commit from bzlmod-migration branch
git ls-remote https://github.com/mmorel-35/examples refs/heads/bzlmod-migration

# Get latest commit from bzlmod branch
git ls-remote https://github.com/mmorel-35/toolshed refs/heads/bzlmod
```

### bazel_dep Declarations

The following bazel_dep declarations are needed for the Envoy ecosystem modules:

```starlark
# Already declared as local_path_override in this repository
bazel_dep(name = "envoy_api")
bazel_dep(name = "envoy_build_config")
bazel_dep(name = "envoy_mobile")

# To be added with git_override
bazel_dep(name = "envoy_examples", dev_dependency = True)  # See recommendations
bazel_dep(name = "envoy_toolshed")
bazel_dep(name = "xds", repo_name = "com_github_cncf_xds")
```

## Critical Blockers

### 🔴 Blocker #1: Missing Version in envoy_examples → envoy_example_wasm_cc

**Status:** Critical - Prevents module resolution

**Description:**
The `envoy_examples` repository has a bazel_dep on `envoy_example_wasm_cc` without specifying a version:

```starlark
# In envoy_examples/MODULE.bazel
bazel_dep(name = "envoy_example_wasm_cc")
local_path_override(
    module_name = "envoy_example_wasm_cc",
    path = "wasm-cc",
)
```

**Error:**
```
ERROR: in module dependency chain <root> -> envoy_examples@_ -> envoy_example_wasm_cc@_: 
bad bazel_dep on module 'envoy_example_wasm_cc' with no version. 
Did you forget to specify a version, or a non-registry override?
```

**Impact:** 
- Bazel module resolution fails
- Cannot proceed with bzlmod testing until resolved

**Solution:**
In the envoy_examples repository (https://github.com/mmorel-35/examples/tree/bzlmod-migration), update MODULE.bazel:

```starlark
# Specify a version even with local_path_override
bazel_dep(name = "envoy_example_wasm_cc", version = "0.0.0")
local_path_override(
    module_name = "envoy_example_wasm_cc",
    path = "wasm-cc",
)
```

The version can be any valid semver (like "0.0.0") since the local_path_override will take precedence.

**Recommended Action:**
Update the envoy_examples bzlmod-migration branch to add version "0.0.0" to the bazel_dep declaration.

### 🔴 Blocker #2: Circular Dependency (envoy ↔ envoy_examples)

**Status:** Critical - Prevents migration

**Description:**
A circular dependency exists between `envoy` and `envoy_examples`:

```
envoy_examples (wasm-cc) 
    → depends on → envoy
        → depends on → envoy_examples (via envoy_dependencies_extension)
```

**Evidence:**
In `envoy/MODULE.bazel`:
```starlark
envoy_deps = use_extension("//bazel:extensions.bzl", "envoy_dependencies_extension")
use_repo(
    envoy_deps,
    ...
    "envoy_examples",
    ...
)
```

In `envoy/bazel/repository_locations.bzl`:
```python
envoy_examples = dict(
    project_name = "envoy_examples",
    project_desc = "Envoy proxy examples",
    project_url = "https://github.com/envoyproxy/examples",
    version = "0.1.4",
    sha256 = "9bb7cd507eb8a090820c8de99f29d9650ce758a84d381a4c63531b5786ed3143",
    strip_prefix = "examples-{version}",
    urls = ["https://github.com/envoyproxy/examples/archive/v{version}.tar.gz"],
    use_category = ["test_only"],  # ← Only used for testing
    ...
)
```

**Impact:** 
- Bazel module resolution will fail due to circular dependency
- Cannot proceed with bzlmod migration until resolved

**Potential Solutions:**

1. **Mark envoy_examples as dev_dependency in envoy (RECOMMENDED)**
   
   Since `envoy_examples` is marked as `use_category = ["test_only"]`, it should be a dev dependency:
   
   ```starlark
   # In envoy MODULE.bazel - if envoy_examples is needed as a module
   bazel_dep(name = "envoy_examples", dev_dependency = True)
   ```
   
   This prevents envoy_examples from being included when envoy is used as a dependency.

2. **Remove envoy_examples dependency from envoy**
   
   If envoy doesn't actually need envoy_examples for its core functionality (only for testing), remove it from the main dependency graph and load it only in CI/test environments.

3. **Split test dependencies into separate extension**
   
   Create a separate module extension for test-only dependencies that is only used when building envoy itself, not when envoy is used as a dependency.

4. **Use archive_override in envoy_examples instead of depending on envoy directly**
   
   Instead of depending on the published envoy module, use archive_override or git_override to get a specific version that doesn't include envoy_examples as a dependency.

**Recommended Action for this repository:**

Since `envoy_examples` is marked as `test_only`, the best approach is to:
- If envoy_examples needs to be declared as bazel_dep, mark it as `dev_dependency = True`
- Consider removing it from the main envoy_dependencies_extension and loading it only in test contexts
- Document that downstream consumers should not depend on envoy_examples through envoy

### 🔴 Blocker #2: LLVM Extension Can Only Be Used by Root Module

**Status:** Critical - Prevents module resolution when envoy is used as a dependency

**Error:**
```
ERROR: Only the root module can use the 'llvm' extension
```

**Description:**
The `envoy` module uses the LLVM toolchain extension in its MODULE.bazel:

```starlark
llvm = use_extension("@toolchains_llvm//toolchain/extensions:llvm.bzl", "llvm")
llvm.toolchain(
    name = "llvm_toolchain",
    llvm_version = "18.1.8",
    cxx_standard = {"": "c++20"},
)
use_repo(llvm, "llvm_toolchain", "llvm_toolchain_llvm")
```

When `envoy` is used as a dependency (not the root module), Bazel's bzlmod system does not allow non-root modules to use this extension because toolchain extensions should be configured by the root module.

**Impact:** 
- Module resolution fails when envoy is used as a dependency
- Cannot test any builds with envoy as a dependency

**Potential Solutions:**

1. **Document LLVM configuration requirements (RECOMMENDED)**
   
   Remove LLVM extension from envoy MODULE.bazel and document that consuming modules must configure toolchains_llvm themselves:
   
   ```starlark
   # In downstream MODULE.bazel that depends on envoy:
   bazel_dep(name = "toolchains_llvm", version = "1.0.0")
   
   llvm = use_extension("@toolchains_llvm//toolchain/extensions:llvm.bzl", "llvm")
   llvm.toolchain(
       name = "llvm_toolchain",
       llvm_version = "18.1.8",
       cxx_standard = {"": "c++20"},
   )
   use_repo(llvm, "llvm_toolchain", "llvm_toolchain_llvm")
   ```

2. **Use a compatibility layer**
   
   Investigate if toolchains_llvm has alternative configuration methods that work for non-root modules.

3. **Keep extension but document the limitation**
   
   Keep the current configuration but document that envoy can only be used as the root module, not as a transitive dependency.

**Recommended Action for this repository:**

The LLVM extension usage should remain in MODULE.bazel since:
- Envoy is typically the root module in builds
- The configuration is essential for Envoy's build requirements
- Document in this file that downstream projects using envoy as a dependency must configure their own LLVM toolchain

**Documentation for downstream consumers:**

If you are using envoy as a dependency in your bzlmod project, you must configure the LLVM toolchain in your root MODULE.bazel with the same settings:
- LLVM version: 18.1.8
- C++ standard: c++20

## Warnings (Non-blocking)

### Rust Cargo Lockfile May Need Update

**Status:** Build-time warning (if building Rust components)

**Potential Error:**
```
The current `lockfile` is out of date for 'dynamic_modules_rust_sdk_crate_index'. 
Please re-run bazel using `CARGO_BAZEL_REPIN=true`
```

**Solution:**
If building Rust components:
```bash
CARGO_BAZEL_REPIN=true bazel build //source/extensions/dynamic_modules/...
git add source/extensions/dynamic_modules/sdk/rust/Cargo.Bazel.lock
git commit -m "Update Rust Cargo lockfiles"
```

### Dependency Version Alignment

When consuming envoy modules, ensure dependency versions align with envoy's requirements. The following versions are used by envoy:

- `rules_cc` @ 0.2.14
- `rules_go` @ 0.59.0
- `rules_python` @ 1.6.3
- `rules_rust` @ 0.67.0
- `protobuf` @ 30.0
- `abseil-cpp` @ 20250814.1

If your project uses different versions, Bazel will automatically resolve to compatible versions, but warnings may appear.

## Dependencies Structure

The envoy bzlmod implementation uses the following module structure:

| Module | Location | Type | Description |
|--------|----------|------|-------------|
| `envoy` | Root repository | Root module | Main envoy module |
| `envoy_api` | `api/` subdirectory | local_path_override | API definitions (protobuf) |
| `envoy_build_config` | `mobile/envoy_build_config/` | local_path_override | Build configuration for mobile |
| `envoy_mobile` | `mobile/` subdirectory | local_path_override | Mobile platform support |
| `envoy_toolshed` | github.com/mmorel-35/toolshed | git_override | Development and CI tooling |
| `envoy_examples` | github.com/mmorel-35/examples | git_override | Example configurations and WASM extensions |
| `xds` | github.com/cncf/xds | git_override | xDS protocol definitions |

## Testing Progress

### Repository Status

- ✅ **envoy** - MODULE.bazel created, extensions implemented
- ✅ **envoy_api** - Local module, MODULE.bazel exists
- ✅ **envoy_mobile** - Local module, MODULE.bazel exists
- ✅ **envoy_build_config** - Local module, MODULE.bazel exists
- ✅ **envoy_toolshed** - bzlmod branch available at github.com/mmorel-35/toolshed
- ⚠️ **envoy_examples** - bzlmod-migration branch available, has circular dependency with envoy

### Build Testing Status

- ✅ Git overrides correctly configured for envoy_toolshed with strip_prefix
- ✅ Git overrides correctly configured for envoy_examples  
- ✅ bazel_dep declarations added
- ✅ Module extensions implemented
- ✅ envoy_examples and envoy_toolshed removed from envoy_dependencies_extension when bzlmod=True
- 🔴 Module dependency graph resolution - **BLOCKED** (envoy_examples needs version on envoy_example_wasm_cc bazel_dep)
- 🔴 LLVM extension limitation - **DOCUMENTED** (only works for root module)
- ⏸️ Circular dependency testing - **PENDING** (waiting for Blocker #1 resolution)
- ⏸️ Full build testing - **PENDING** (waiting for all blockers resolution)

## Next Steps

### Immediate Actions (envoy_examples Repository)

1. **[CRITICAL] Fix missing version in bazel_dep**
   - In envoy_examples/MODULE.bazel, add version to envoy_example_wasm_cc:
   ```starlark
   bazel_dep(name = "envoy_example_wasm_cc", version = "0.0.0")
   ```

### Immediate Actions (This Repository)

2. **[COMPLETED] Add git_override entries**
   - ✅ Added bazel_dep and git_override for envoy_examples (with dev_dependency = True)
   - ✅ Added bazel_dep and git_override for envoy_toolshed (with strip_prefix)
   - ✅ Removed envoy_examples and envoy_toolshed from use_repo when bzlmod=True

3. **[COMPLETED] Document LLVM requirements**
   - ✅ Documented that LLVM extension only works for root modules
   - ✅ Document workarounds for downstream consumers

4. **[PENDING] Address circular dependency with envoy_examples**
   - After Blocker #1 is fixed, test if circular dependency exists
   - Option A: Verify dev_dependency = True prevents circular dependency
   - Option B: If circular dependency still exists, consider removing envoy dependency from envoy_examples

5. **[PENDING] Test module resolution**
   - Run `bazel mod graph --enable_bzlmod` to visualize dependency graph
   - Verify no circular dependencies
   - Check for version conflicts

### For envoy_examples bzlmod-migration branch

5. **Remove or restructure envoy dependency**
   - If envoy_examples needs envoy, use archive_override to break the cycle
   - Consider splitting examples into those that need envoy and those that don't
   - Update dependency versions to match envoy's requirements

6. **Test builds** (after blockers resolved)
   - Test `bazel build //wasm-cc:envoy_filter_http_wasm_example.wasm`
   - Test other example builds
   - Verify CI compatibility

### For envoy_toolshed bzlmod branch

7. **Verify toolshed integration**
   - Test that git_override works correctly
   - Verify no missing dependencies
   - Check build compatibility with envoy

## Recommendations

### For envoy (this repository)

1. **Treat envoy_examples as dev-only dependency**
   - Since it's marked as `test_only`, it should not be a runtime dependency
   - If declaring as bazel_dep, use `dev_dependency = True`
   - This prevents circular dependency issues

2. **Document LLVM toolchain requirements**
   - Clearly state that envoy must be the root module OR
   - Provide documentation for downstream projects to configure LLVM themselves

3. **Consider splitting test dependencies**
   - Create a separate dev_dependencies extension for test-only dependencies
   - This makes it clear what's needed for using envoy vs. developing envoy

### For envoy_examples

1. **Break circular dependency**
   - Don't depend on envoy as a regular bazel_dep
   - Use archive_override or git_override if specific envoy version is needed
   - Document which version of envoy the examples are compatible with

2. **Update dependency versions**
   - Align with envoy's requirements (rules_cc, rules_go, etc.)
   - This reduces version conflict warnings

### For envoy_toolshed

1. **Verify bzlmod compatibility**
   - Ensure all toolshed tools work with bzlmod mode
   - Test integration with envoy builds
   - Document any bzlmod-specific requirements

## References

- **Envoy bzlmod migration branch**: https://github.com/mmorel-35/envoy/tree/bzlmod-migration
- **Examples bzlmod migration branch**: https://github.com/mmorel-35/examples/tree/bzlmod-migration
- **Toolshed bzlmod branch**: https://github.com/mmorel-35/toolshed/tree/bzlmod
- **Bazel bzlmod documentation**: https://bazel.build/external/module
- **Module extensions documentation**: https://bazel.build/external/extension
- **Bazel Central Registry**: https://registry.bazel.build/

## Migration Timeline

### Phase 1: Module Structure (Current)
- ✅ Create MODULE.bazel files
- ✅ Implement module extensions
- ✅ Set up local_path_override for internal modules
- 🔄 Add git_override for external bzlmod branches

### Phase 2: Dependency Resolution
- ⏸️ Resolve circular dependencies
- ⏸️ Fix version conflicts
- ⏸️ Test module dependency graph

### Phase 3: Build Validation
- ⏸️ Test Envoy core builds
- ⏸️ Test Envoy mobile builds
- ⏸️ Test example builds
- ⏸️ Validate all CI pipelines

### Phase 4: Production Readiness
- ⏸️ Performance testing
- ⏸️ Documentation updates
- ⏸️ Migration guide for downstream users
- ⏸️ Deprecation plan for WORKSPACE mode

## Appendix: Common Bzlmod Issues and Solutions

### How to Update git_override Commits

When the bzlmod migration branches are updated, you'll need to update the commit hashes in MODULE.bazel:

```bash
# Get latest commit hash from envoy_examples bzlmod-migration branch
EXAMPLES_COMMIT=$(git ls-remote https://github.com/mmorel-35/examples refs/heads/bzlmod-migration | cut -f1)
echo "envoy_examples: $EXAMPLES_COMMIT"

# Get latest commit hash from envoy_toolshed bzlmod branch  
TOOLSHED_COMMIT=$(git ls-remote https://github.com/mmorel-35/toolshed refs/heads/bzlmod | cut -f1)
echo "envoy_toolshed: $TOOLSHED_COMMIT"

# Update MODULE.bazel with the new commit hashes
# Then test with: bazel mod graph --enable_bzlmod
```

After updating commits, always test module resolution before committing:
```bash
bazel mod graph --enable_bzlmod
```

### Issue: "Only the root module can use extension X"

**Cause:** Some extensions (like toolchain configurations) can only be used by the root module.

**Solution:** 
- For library modules: Remove the extension and document requirements
- For application modules: Keep the extension (you'll always be the root)

### Issue: Circular dependencies

**Cause:** Module A depends on B, and B depends on A.

**Solution:**
- Use `dev_dependency = True` for test-only dependencies
- Use `archive_override` or `git_override` to break the cycle
- Restructure modules to remove circular dependencies

### Issue: Version conflicts

**Cause:** Different modules require different versions of the same dependency.

**Solution:**
- Bazel will select the highest compatible version
- Use `single_version_override` if you need to force a specific version
- Update modules to use compatible version ranges

### Issue: Missing dependencies in transitive deps

**Cause:** Dependencies not properly declared in MODULE.bazel files.

**Solution:**
- Ensure all direct dependencies are declared
- Use `bazel mod graph` to visualize dependencies
- Check that all `use_repo()` calls match the actual usage

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

**Note on Repository References:**
This implementation uses work-in-progress bzlmod migration branches from mmorel-35 forks as specified in the migration requirements:
- https://github.com/mmorel-35/envoy/tree/bzlmod-migration
- https://github.com/mmorel-35/examples/tree/bzlmod-migration
- https://github.com/mmorel-35/toolshed/tree/bzlmod

These are temporary development branches. Once the migration is complete and tested, these should be:
1. Merged into the official envoyproxy organization repositories
2. Updated in MODULE.bazel to point to official envoyproxy repositories
3. Eventually published to Bazel Central Registry (BCR) if appropriate

## Changes Made in This Repository

This repository (envoy) has been updated with the following bzlmod migration changes:

### ✅ Completed Changes

1. **Added bazel_dep declarations for ecosystem modules:**
   - `bazel_dep(name = "envoy_toolshed")` - Runtime dependency
   - `bazel_dep(name = "envoy_examples", dev_dependency = True, version = "0.1.5-dev")` - Dev-only dependency
   - All local modules now have versions: envoy_api, envoy_build_config, envoy_mobile (1.36.4-dev)

2. **Added git_override entries to use bzlmod migration branches:**
   - `envoy_examples`: Points to https://github.com/mmorel-35/examples bzlmod-migration branch (commit 0b56dee5)
   - `envoy_example_wasm_cc`: Points to https://github.com/mmorel-35/examples bzlmod-migration branch with `strip_prefix = "wasm-cc"` (commit 0b56dee5)
   - `envoy_toolshed`: Points to https://github.com/mmorel-35/toolshed bzlmod branch with `strip_prefix = "bazel"` (commit 192c4fca)

3. **Updated bazel/repositories.bzl:**
   - Wrapped `envoy_examples` and `envoy_toolshed` http_archive calls with `if not bzlmod:` condition
   - This prevents double-loading when bzlmod is enabled

4. **Removed from envoy_dependencies_extension use_repo:**
   - Removed `envoy_examples` and `envoy_toolshed` from use_repo() call
   - These are now loaded as bazel_dep modules instead of through the extension

5. **Created comprehensive documentation:**
   - This file (docs/bzlmod_migration.md) documents all blockers, recommendations, and migration status

### 🔄 Latest Updates (2025-12-10)

**Update #3 (commit 9423333):**
- ✅ Updated git_override commits to ed43f119 (examples) and 6b035f94 (toolshed)
- ✅ **Blocker #2 RESOLVED**: LLVM extension removed from envoy_example_wasm_cc
- ✅ **Blocker #3 RESOLVED**: LLVM extension removed from envoy_toolshed  
- ✅ **Blocker #4 RESOLVED**: Fixed envoy_mobile kotlin_formatter/robolectric use_repo issue
- ✅ **Module resolution SUCCESS**: `bazel mod graph --enable_bzlmod` completes without errors
- ⚠️ Non-blocking warnings: Maven version conflicts (gson, error_prone_annotations, guava)

**Update #2 (commit 066ef05):**
- Updated git_override commits to 0b56dee5 (examples) and 192c4fca (toolshed)
- Identified Blockers #2 and #3 (LLVM extension usage)
- Tested module resolution

**Update #1 (initial commits):**
- Added git_override for envoy_examples and envoy_toolshed
- Added version to envoy module
- Created comprehensive documentation

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
git_override(
    module_name = "envoy_examples",
    commit = "ed43f119108bce30c2c148227be6b79f360adecc",  # bzlmod-migration branch
    remote = "https://github.com/mmorel-35/examples",
)

# Envoy example wasm-cc - bzlmod migration branch
git_override(
    module_name = "envoy_example_wasm_cc",
    commit = "ed43f119108bce30c2c148227be6b79f360adecc",  # bzlmod-migration branch
    remote = "https://github.com/mmorel-35/examples",
    strip_prefix = "wasm-cc",
)

# Envoy toolshed - bzlmod migration branch
# Note: strip_prefix points to bazel/ subdirectory where MODULE.bazel is located
git_override(
    module_name = "envoy_toolshed",
    commit = "6b035f9418c0512c95581736ce77d9f39e99e703",  # bzlmod branch
    remote = "https://github.com/mmorel-35/toolshed",
    strip_prefix = "bazel",
)
```

**Commit References:**
- envoy_examples: `ed43f119108bce30c2c148227be6b79f360adecc` from bzlmod-migration branch (updated 2025-12-10)
- envoy_example_wasm_cc: `ed43f119108bce30c2c148227be6b79f360adecc` from bzlmod-migration branch (updated 2025-12-10)
- envoy_toolshed: `6b035f9418c0512c95581736ce77d9f39e99e703` from bzlmod branch (updated 2025-12-10)

**Update History:**
- 2025-12-10 (commit 9423333): Updated to ed43f119 (examples) and 6b035f94 (toolshed) - LLVM extensions removed
- 2025-12-10 (commit 066ef05): Updated to 0b56dee5 (examples) and 192c4fca (toolshed) - versions added
- Initial: 1ceb95e9 (examples) and d718b38e (toolshed)

These commits include all fixes for Blockers #1-4. To update to newer commits:
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

### ✅ Blocker #1: Missing Version in envoy_examples → envoy_example_wasm_cc - RESOLVED

**Status:** ✅ RESOLVED - Version added in commit 0b56dee5

**Description:**
The `envoy_examples` repository previously had a bazel_dep on `envoy_example_wasm_cc` without specifying a version.

**Solution Applied:**
Updated in https://github.com/mmorel-35/examples/commit/0b56dee5:

```starlark
# envoy_examples/MODULE.bazel
module(
    name = "envoy_examples",
    version = "0.1.5-dev",
)
bazel_dep(name = "envoy_example_wasm_cc", version = "0.1.5-dev")
local_path_override(
    module_name = "envoy_example_wasm_cc",
    path = "wasm-cc",
)
```

All envoy* modules now have versions:
- envoy: 1.36.4-dev
- envoy_api: 1.36.4-dev
- envoy_build_config: 1.36.4-dev
- envoy_mobile: 1.36.4-dev
- envoy_examples: 0.1.5-dev
- envoy_example_wasm_cc: 0.1.5-dev
- envoy_toolshed: 0.3.8-dev

### ✅ Blocker #2: LLVM Extension in envoy_example_wasm_cc - RESOLVED

**Status:** ✅ RESOLVED - LLVM extension removed in commit ed43f119

**Description:**
The `envoy_example_wasm_cc` MODULE.bazel was using the LLVM extension, which can only be used by the root module.

**Solution Applied:**
Updated in https://github.com/mmorel-35/examples/commit/ed43f119:

Removed LLVM extension usage from wasm-cc/MODULE.bazel.

See: https://github.com/mmorel-35/examples/blob/bzlmod-migration/docs/bzlmod_migration.md

### ✅ Blocker #3: LLVM Extension in envoy_toolshed - RESOLVED

**Status:** ✅ RESOLVED - LLVM extension removed in commit 6b035f94

**Description:**
The `envoy_toolshed` MODULE.bazel was using the LLVM extension.

**Solution Applied:**
Updated in https://github.com/mmorel-35/toolshed/commit/6b035f94:

Removed LLVM extension and toolchains_llvm dependency from bazel/MODULE.bazel.

**Note:** LLVM sanitizer library builds (e.g., `//compile:cxx_msan`) are only available in WORKSPACE mode, not bzlmod mode.

See: https://github.com/mmorel-35/toolshed/blob/bzlmod/docs/bzlmod_migration.md

### ✅ Blocker #4: envoy_mobile kotlin_formatter/robolectric in use_repo - RESOLVED

**Status:** ✅ RESOLVED - Removed from use_repo in commit 9423333

**Description:**
The `mobile/MODULE.bazel` was trying to import `kotlin_formatter` and `robolectric` from the envoy_mobile_dependencies extension, but these dependencies are only loaded when `not bzlmod`.

**Error:**
```
ERROR: module extension "envoy_mobile_dependencies" does not generate repository "kotlin_formatter"
```

**Solution Applied:**
Removed `kotlin_formatter` and `robolectric` from use_repo in mobile/MODULE.bazel (commit 9423333).

These tools are Kotlin/Android development dependencies that are only needed in WORKSPACE mode.
Update envoy_toolshed/bazel/MODULE.bazel in the bzlmod branch to remove LLVM extension usage.

### 🟡 Blocker #5: Circular Dependency (envoy ↔ envoy_examples)

**Status:** Mitigated - Prevented via dev_dependency configuration

**Description:**
A potential circular dependency exists between `envoy` and `envoy_examples`:

```
envoy_examples (wasm-cc) 
    → depends on → envoy
        → depends on → envoy_examples (via envoy_dependencies_extension)
```

**Evidence:**
In `envoy/MODULE.bazel`:
```starlark
bazel_dep(name = "envoy_examples", dev_dependency = True)

git_override(
    module_name = "envoy_examples",
    commit = "1ceb95e9c9c8b1892d0c14a1ba4c42216348831d",
    remote = "https://github.com/mmorel-35/examples",
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

In `envoy_examples/wasm-cc/MODULE.bazel`:
```starlark
bazel_dep(name = "envoy")

git_override(
    module_name = "envoy",
    commit = "4fc5c5cd8a2aec2a51fd21462bbd648d92d0889e",
    remote = "https://github.com/mmorel-35/envoy",
)
```

This creates a circular dependency: envoy → envoy_examples (dev) → wasm-cc → envoy

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
- ✅ **IMPLEMENTED**: envoy_examples is declared with `dev_dependency = True`
- ✅ **IMPLEMENTED**: This prevents envoy_examples from being included when envoy is used as a dependency
- Document that downstream consumers should not depend on envoy_examples through envoy

**Status of Circular Dependency:**
The circular dependency has been **mitigated** by marking envoy_examples as `dev_dependency = True`. This means:
- When envoy is built standalone (as root module), envoy_examples is loaded
- When envoy is used as a dependency by another project, envoy_examples is NOT loaded
- The wasm-cc example in envoy_examples uses git_override to point to mmorel-35/envoy (bzlmod-migration branch)

This configuration allows testing without creating a true circular dependency in the module graph.

### 📝 Blocker #6: LLVM Extension Can Only Be Used by Root Module (Documentation)

**Status:** Documented - Expected behavior, not a blocker for envoy as root module

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

### Version Conflicts in envoy_examples/wasm-cc

Several dependency version mismatches exist between envoy and wasm-cc MODULE.bazel files:

| Dependency | envoy version | wasm-cc version | Status |
|------------|---------------|-----------------|--------|
| rules_cc | 0.2.14 | 0.1.1 | ⚠️ Mismatch |
| rules_go | 0.59.0 | 0.53.0 | ⚠️ Mismatch |
| rules_python | 1.6.3 | 1.4.1 | ⚠️ Mismatch |
| rules_rust | 0.67.0 | 0.56.0 | ⚠️ Mismatch |
| toolchains_llvm | git_override (commit fb29f3d) | 1.4.0 | ⚠️ Different source |

**Solution:**
Update `wasm-cc/MODULE.bazel` to use compatible versions:

```starlark
# Update to match envoy's requirements:
bazel_dep(name = "rules_cc", version = "0.2.14")
bazel_dep(name = "rules_go", version = "0.59.0", repo_name = "io_bazel_rules_go")
bazel_dep(name = "rules_python", version = "1.6.3")
bazel_dep(name = "rules_rust", version = "0.67.0")

# Remove toolchains_llvm bazel_dep if using git_override from envoy
# Or align with envoy's git_override
```

Bazel will automatically resolve to the highest compatible version, but warnings may appear during module resolution.

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

- ✅ Git overrides updated to latest commits (ed43f119 for examples, 6b035f94 for toolshed)
- ✅ All critical blockers (#1-4) RESOLVED
- ✅ Versions added to all envoy* modules
- ✅ bazel_dep declarations include versions
- ✅ Module extensions implemented
- ✅ envoy_examples and envoy_toolshed removed from envoy_dependencies_extension when bzlmod=True
- ✅ Added git_override for envoy_example_wasm_cc module
- ✅ Removed kotlin_formatter and robolectric from envoy_mobile use_repo
- ✅ Module dependency graph resolution - **SUCCESS** (`bazel mod graph --enable_bzlmod` completes)
- 🟡 Circular dependency - **MITIGATED** (dev_dependency = True - Blocker #5)
- 📝 LLVM extension limitation - **DOCUMENTED** (only works for root module - Blocker #6)
- ⚠️ Maven version warnings - **NON-BLOCKING** (gson, error_prone_annotations, guava version conflicts)
- ⏸️ Full build testing - **READY** (can now proceed with build tests)

## Next Steps

### Completed Actions

1. **[✅ COMPLETED] Update git_override commits**
   - Updated to ed43f119 for envoy_examples and envoy_example_wasm_cc
   - Updated to 6b035f94 for envoy_toolshed

2. **[✅ COMPLETED] Resolve LLVM extension blockers**
   - Blocker #2: LLVM extension removed from envoy_example_wasm_cc
   - Blocker #3: LLVM extension removed from envoy_toolshed

3. **[✅ COMPLETED] Fix envoy_mobile kotlin_formatter blocker**
   - Blocker #4: Removed kotlin_formatter and robolectric from use_repo

4. **[✅ COMPLETED] Module resolution testing**
   - `bazel mod graph --enable_bzlmod` completes successfully
   - All critical blockers resolved

### Ready for Next Phase

5. **[READY] Build testing with bzlmod**
   - Test core envoy builds with `--enable_bzlmod`
   - Test envoy_mobile builds
   - Test example builds (wasm-cc)
   - Identify any build-time issues

6. **[READY] CI/CD integration**
   - Update CI workflows to test with bzlmod
   - Ensure both WORKSPACE and bzlmod modes work
   - Add bzlmod-specific test targets

### Ongoing Monitoring

7. **Maven version warnings (non-blocking)**
   - Monitor version conflicts: gson (2.10.1 vs 2.8.9), error_prone_annotations (2.23.0 vs 2.5.1), guava (32.0.1-jre vs 33.0.0-jre)
   - Consider adding explicit version pins if issues arise
   - Can be addressed with `known_contributing_modules` attribute if needed

### For envoy_examples bzlmod-migration branch

7. **[COMPLETED] Fix bazel_dep version issue**
   - ✅ Version "0.1.5-dev" added to envoy_example_wasm_cc bazel_dep in commit 0b56dee5
   
8. **[CRITICAL] Remove LLVM extension from wasm-cc**
   - Remove LLVM extension usage from wasm-cc/MODULE.bazel (see step 1 above)
   - This is Blocker #2 that prevents module resolution

9. **[PENDING] Check for circular dependency with envoy**
   - After fixing Blockers #2 and #3, verify the circular dependency is properly mitigated
   - The dev_dependency = True should prevent issues

10. **[PENDING] Update dependency versions** (after blockers resolved)
    - Align rules_cc, rules_go, rules_python, rules_rust versions with envoy
    - Current mismatches documented in Version Conflicts section

11. **[PENDING] Test builds** (after blockers resolved)
    - Test `bazel build //wasm-cc:envoy_filter_http_wasm_example.wasm`
    - Test other example builds
    - Verify CI compatibility

### For envoy_toolshed bzlmod branch

12. **[CRITICAL] Remove LLVM extension from envoy_toolshed**
    - Remove LLVM extension usage from envoy_toolshed/bazel/MODULE.bazel (see step 2 above)
    - This is Blocker #3 that prevents module resolution
    - Consider if toolchains_llvm bazel_dep is actually needed

13. **[PENDING] Verify toolshed integration** (after blocker resolved)
    - Test that git_override works correctly after LLVM extension removal
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

### Phase 5: Official Repository Migration
- ⏸️ Merge bzlmod branches to official envoyproxy repositories
- ⏸️ Update git_override entries to point to envoyproxy organization
- ⏸️ Consider publishing to Bazel Central Registry (BCR)
- ⏸️ Archive temporary development branches

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

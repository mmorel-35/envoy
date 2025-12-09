# Aspect Bazel Lib Toolchains

This document lists all toolchains available in [aspect_bazel_lib](https://github.com/bazel-contrib/bazel-lib) and their current usage in Envoy.

Envoy currently uses aspect_bazel_lib version **2.21.2** (defined in `MODULE.bazel`).

## Available Toolchains

### 1. jq - JSON Processor

**Status:** ✅ **CURRENTLY USED**

- **Description:** Command-line JSON processor for querying and manipulating JSON data
- **Envoy Version:** 1.7 (defined in `bazel/dependency_imports.bzl`)
- **aspect_bazel_lib Default:** 1.7
- **Registration:** `register_jq_toolchains()` in `bazel/dependency_imports.bzl`
- **Usage in Envoy:**
  - `tools/base/envoy_python.bzl` - JSON generation and manipulation
  - `bazel/repo.bzl` - Repository operations
  - `tools/jq/BUILD` - JQ tools
- **Documentation:** https://github.com/stedolan/jq

### 2. yq - YAML Processor

**Status:** ✅ **CURRENTLY USED**

- **Description:** Command-line YAML processor for querying and manipulating YAML data
- **Envoy Version:** 4.24.4 (defined in `bazel/dependency_imports.bzl`)
- **aspect_bazel_lib Default:** 4.24.4
- **Registration:** `register_yq_toolchains()` in `bazel/dependency_imports.bzl`
- **Usage in Envoy:**
  - `tools/base/envoy_python.bzl` - YAML to JSON conversion and manipulation
- **Documentation:** https://github.com/mikefarah/yq

### 3. zstd - Compression Tool

**Status:** ⚠️ **DIFFERENT USE CASE**

- **Description:** Zstandard compression algorithm implementation
- **Notes:**
  - Envoy uses `bazel_dep(name = "zstd", version = "1.5.7")` in `MODULE.bazel` for the **C library**
  - aspect_bazel_lib provides zstd toolchain for the **CLI tool**
  - These serve different purposes and are not mutually exclusive
  - Envoy also uses `com_github_qat_zstd` for Intel QuickAssist Technology ZSTD Plugin
- **Recommendation:** No changes needed - both can coexist
- **Documentation:** https://github.com/facebook/zstd

### 4. bsdtar - BSD Tar Archival Tool

**Status:** ❌ **NOT CURRENTLY USED**

- **Description:** Cross-platform tar implementation with consistent behavior
- **Potential Use Cases:**
  - Archive operations in build process
  - Cross-platform archive handling
  - Consistent tar behavior across different host platforms
- **Registration:** `register_tar_toolchains()`
- **Documentation:** Part of libarchive project

### 5. bats - Bash Automated Testing System

**Status:** ❌ **NOT CURRENTLY USED**

- **Description:** TAP-compliant testing framework for Bash scripts
- **Potential Use Cases:**
  - Testing shell scripts in Envoy
  - CI/CD shell script validation
  - Build script testing
- **Registration:** `register_bats_toolchains()`
- **Includes:**
  - bats-core (v1.10.0)
  - bats-support (v0.3.0)
  - bats-assert (v2.1.0)
  - bats-file (v0.4.0)
- **Documentation:** https://github.com/bats-core/bats-core

### 6. coreutils - Core Utilities

**Status:** ❌ **NOT CURRENTLY USED**

- **Description:** Rust-based reimplementation of GNU coreutils (uutils/coreutils)
- **Potential Use Cases:**
  - Cross-platform build scripts with consistent behavior
  - Hermetic builds with controlled utility versions
  - Platform-independent file operations
- **Registration:** `register_coreutils_toolchains()`
- **Documentation:** https://github.com/uutils/coreutils

### 7. copy_directory

**Status:** ❌ **NOT CURRENTLY USED**

- **Description:** Built-in aspect_bazel_lib tool for copying directories
- **Potential Use Cases:**
  - Efficient directory copying operations
  - Build artifact organization
  - Distribution packaging
- **Registration:** `register_copy_directory_toolchains()`
- **Documentation:** https://docs.aspect.build/rulesets/aspect_bazel_lib/docs/copy_directory/

### 8. copy_to_directory

**Status:** ❌ **NOT CURRENTLY USED**

- **Description:** Built-in aspect_bazel_lib tool for assembling directories from multiple sources
- **Potential Use Cases:**
  - Building distribution packages
  - Assembling release artifacts
  - Creating directory trees from multiple inputs
- **Registration:** `register_copy_to_directory_toolchains()`
- **Documentation:** https://docs.aspect.build/rulesets/aspect_bazel_lib/docs/copy_to_directory/

### 9. expand_template

**Status:** ❌ **NOT CURRENTLY USED**

- **Description:** Built-in aspect_bazel_lib tool for template expansion
- **Potential Use Cases:**
  - Configuration file generation
  - Build-time template processing
  - Version stamping
- **Registration:** `register_expand_template_toolchains()`
- **Documentation:** https://docs.aspect.build/rulesets/aspect_bazel_lib/docs/expand_template/

## Summary

### Currently Integrated (2 toolchains)
- ✅ **jq** (v1.7) - JSON processing
- ✅ **yq** (v4.24.4) - YAML processing

### Different Use Cases (1 toolchain)
- ⚠️ **zstd** - Envoy uses the library, aspect_bazel_lib provides CLI tool

### Available for Future Use (6 toolchains)
- ❌ **bsdtar** - Archive operations
- ❌ **bats** - Shell script testing
- ❌ **coreutils** - Cross-platform utilities
- ❌ **copy_directory** - Directory copying
- ❌ **copy_to_directory** - Directory assembly
- ❌ **expand_template** - Template expansion

## Recommendations

1. **jq and yq** - Already properly integrated, no changes needed
2. **zstd** - No conflict between library (bazel_dep) and CLI tool (aspect_bazel_lib)
3. **Future considerations** - Evaluate the 6 unused toolchains for potential integration based on specific needs

## Additional Resources

- **aspect_bazel_lib GitHub:** https://github.com/bazel-contrib/bazel-lib
- **aspect_bazel_lib Documentation:** https://docs.aspect.build/rulesets/aspect_bazel_lib
- **Bazel Central Registry:** https://registry.bazel.build/modules/aspect_bazel_lib

## Version Information

- **Current aspect_bazel_lib version in Envoy:** 2.21.2
- **Location:** `MODULE.bazel`
- **Dependency imports:** `bazel/dependency_imports.bzl`
- **Repository locations:** `bazel/repository_locations.bzl`

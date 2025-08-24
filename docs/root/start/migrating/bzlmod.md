# Bzlmod Migration for Envoy - COMPLETE

This document describes the **completed** bzlmod migration implemented for Envoy to achieve full Bazel 8.0 compatibility.

## Migration Status: ✅ COMPLETE

**All WORKSPACE.bzlmod files have been eliminated** and all repository setup has been migrated to dedicated module extensions with per-module organization.

### ✅ Pure Bzlmod Architecture Achieved

All modules are now fully bzlmod-compatible:

- ✅ Main module: 5 extensions in `@envoy//bazel/extensions/`
- ✅ API module: 1 extension in `@envoy_api//bazel/extensions/`  
- ✅ Mobile module: 6 extensions in `@envoy_mobile//bazel/extensions/`
- ✅ Build config module: Clean MODULE.bazel only
- ✅ **No WORKSPACE.bzlmod files** - Pure bzlmod architecture achieved

### Multi-Component Architecture

```
├── MODULE.bazel                              # Root Envoy dependencies
├── api/
│   └── MODULE.bazel                          # API module
├── mobile/
│   ├── MODULE.bazel                          # Mobile dependencies  
│   └── envoy_build_config/
│       └── MODULE.bazel                      # Build config
```

### 🔧 Complete Bzlmod Migration

**All dependency functions migrated to 12 dedicated per-module extensions:**

**Main Module Extensions:**
- `dependencies` - Core dependency definitions
- `dependencies_extra` - Second-stage dependencies
- `dependency_imports` - Toolchain imports
- `dependency_imports_extra` - Additional imports  
- `repo` - Repository metadata

**API Module Extensions:**
- `api_dependencies` - API-specific dependencies

**Mobile Module Extensions:**
- `mobile` - Mobile dependencies (Swift, Kotlin, Android)
- `repos` - Mobile repository setup
- `toolchains` - Mobile toolchain registration
- `android` - Android SDK/NDK configuration
- `android_workspace` - Android workspace setup
- `workspace` - Xcode and provisioning setup
### Key Benefits

**✅ Bazel 8.0 Ready**: Pure bzlmod architecture with zero WORKSPACE dependencies across all modules

**✅ Per-Module Organization**: Each module manages its own extensions in dedicated `bazel/extensions/` directories

**✅ Simplified Naming**: Clear extension names without redundant prefixes leveraging directory structure for context

**✅ Standards Compliance**: Uses upstream extensions where available (rules_python) while maintaining custom extensions only where necessary

**✅ Third_party Compatibility**: Incremental migration layer allows gradual transition from //external: to @repo//:target patterns

## Extension Organization

### Main Module (@envoy//bazel/extensions/)
- `dependencies.bzl` - Core dependency definitions with patches
- `dependencies_extra.bzl` - Second-stage dependencies  
- `dependency_imports.bzl` - Toolchain imports and registrations
- `dependency_imports_extra.bzl` - Additional dependency imports
- `repo.bzl` - Repository metadata setup

### API Module (@envoy_api//bazel/extensions/)
- `api_dependencies.bzl` - API-specific dependencies

### Mobile Module (@envoy_mobile//bazel/extensions/)  
- `mobile.bzl` - Mobile-specific dependencies (Swift, Kotlin, Android)
- `repos.bzl` - Mobile repository setup
- `toolchains.bzl` - Mobile toolchain registration
- `android.bzl` - Android SDK/NDK configuration
- `android_workspace.bzl` - Android workspace setup
- `workspace.bzl` - Xcode and provisioning setup

## Usage in MODULE.bazel

### Main Module
```starlark
# Core extensions
envoy_deps = use_extension("//bazel/extensions:dependencies.bzl", "dependencies")
envoy_deps_extra = use_extension("//bazel/extensions:dependencies_extra.bzl", "dependencies_extra")
envoy_imports = use_extension("//bazel/extensions:dependency_imports.bzl", "dependency_imports")
envoy_imports_extra = use_extension("//bazel/extensions:dependency_imports_extra.bzl", "dependency_imports_extra")
envoy_repo_setup = use_extension("//bazel/extensions:repo.bzl", "repo")

# Upstream Python extensions (replacing custom envoy_python_dependencies_ext)
python = use_extension("@rules_python//python/extensions:python.bzl", "python")
pip = use_extension("@rules_python//python/extensions:pip.bzl", "pip")
```

### API Module
```starlark
envoy_api_deps = use_extension("//bazel/extensions:api_dependencies.bzl", "api_dependencies")
```

### Mobile Module
```starlark
# Mobile-specific extensions
envoy_mobile_deps = use_extension("//bazel/extensions:mobile.bzl", "mobile")
envoy_mobile_repos = use_extension("//bazel/extensions:repos.bzl", "repos")
envoy_mobile_toolchains = use_extension("//bazel/extensions:toolchains.bzl", "toolchains")
envoy_android_config = use_extension("//bazel/extensions:android.bzl", "android")
envoy_android_workspace = use_extension("//bazel/extensions:android_workspace.bzl", "android_workspace")
envoy_mobile_workspace = use_extension("//bazel/extensions:workspace.bzl", "workspace")
```

## Legacy Support

### WORKSPACE Compatibility
The original functions in `bazel/*.bzl` files are preserved for WORKSPACE compatibility:
- `envoy_dependencies()` in `bazel/repositories.bzl`
- `envoy_api_binding()` in `bazel/api_binding.bzl`  
- Mobile functions in `mobile/bazel/*.bzl`

These files maintain their original API for projects still using WORKSPACE mode, while bzlmod uses the dedicated extensions.

### Third_party Migration Layer
The `third_party/BUILD.bazel` compatibility layer provides aliases for legacy //external: references:

```starlark
alias(name = "zlib", actual = "@zlib//:zlib")
alias(name = "ssl", actual = "@envoy//bazel:boringssl")
```

This enables incremental migration from bind() patterns to direct @repo//:target references.

## Validation Commands

### Check bzlmod dependency graph
```bash
bazel mod graph
```

### Validate MODULE.bazel extensions
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

1. **Module resolution errors**: Check extension paths match actual file locations
2. **Missing dependencies**: Verify all required modules are listed in MODULE.bazel
3. **Extension loading errors**: Ensure extension functions are properly exported

### Getting Help

- Check the [Bazel Community Slack](https://slack.bazel.build/) #bzlmod channel
- Review existing migration examples in the BCR
- Open issues in the [bazel-central-registry](https://github.com/bazelbuild/bazel-central-registry) for missing modules
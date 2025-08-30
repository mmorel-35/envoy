# Envoy Bzlmod Migration: Recommendations for Best Practices Compliance

This document provides specific recommendations for improving Envoy's bzlmod implementation to better align with [official Bazel best practices](https://bazel.build/external/migration).

## Executive Summary

Envoy has made excellent progress on bzlmod migration with 47+ dependencies moved to direct MODULE.bazel declarations and a well-structured extension system. However, several opportunities exist to reduce complexity and better align with Bazel's recommended patterns.

## Current State Assessment

### ✅ Strengths
- **Strong BCR adoption**: 47+ clean dependencies using direct bazel_dep declarations
- **Organized extensions**: Clear separation by module (main, API, mobile)
- **Upstream integration**: Using @rules_python extensions instead of custom ones
- **Hybrid compatibility**: Maintains WORKSPACE support during transition

### ⚠️ Areas for Improvement
- **Extension proliferation**: 12 extensions could be consolidated
- **WORKSPACE.bzlmod remnants**: Still present though minimal
- **Patch dependency**: Heavy reliance on custom patches preventing BCR adoption
- **Extension granularity**: Some extensions are too broad, others too narrow

## Priority Recommendations

### 1. HIGH PRIORITY: Consolidate Extension Architecture

**Current State**: 12 separate extensions across modules
**Recommendation**: Reduce to 6-8 focused extensions

#### Main Module Consolidation
```starlark
# CURRENT (4 extensions)
envoy_deps = use_extension("//bazel/extensions:dependencies.bzl", "dependencies")
envoy_deps_extra = use_extension("//bazel/extensions:dependencies_extra.bzl", "dependencies_extra")
envoy_imports = use_extension("//bazel/extensions:dependency_imports.bzl", "dependency_imports")
envoy_imports_extra = use_extension("//bazel/extensions:dependency_imports_extra.bzl", "dependency_imports_extra")

# RECOMMENDED (2 extensions)
envoy_core = use_extension("//bazel/extensions:core.bzl", "core")
envoy_toolchains = use_extension("//bazel/extensions:toolchains.bzl", "toolchains")
```

**Benefits**:
- Simplified dependency graph
- Easier maintenance and debugging
- Follows bzlmod principle of "minimal necessary extensions"

#### Mobile Module Streamlining
```starlark
# CURRENT (6 extensions)
envoy_mobile_deps = use_extension("//bazel/extensions:mobile.bzl", "mobile")
envoy_mobile_repos = use_extension("//bazel/extensions:repos.bzl", "repos")
envoy_mobile_toolchains = use_extension("//bazel/extensions:toolchains.bzl", "toolchains")
envoy_android_config = use_extension("//bazel/extensions:android.bzl", "android")
envoy_android_workspace = use_extension("//bazel/extensions:android_workspace.bzl", "android_workspace")
envoy_mobile_workspace = use_extension("//bazel/extensions:workspace.bzl", "workspace")

# RECOMMENDED (2 extensions)
mobile_deps = use_extension("//bazel/extensions:mobile_deps.bzl", "mobile_deps")
mobile_toolchains = use_extension("//bazel/extensions:mobile_toolchains.bzl", "mobile_toolchains")
```

### 2. HIGH PRIORITY: Upstream Patch Contributions

**Current Issue**: 33+ dependencies require custom patches, preventing BCR adoption

**Recommended Actions**:

#### Immediate (Next 3 months)
1. **protobuf patches**: Submit arena.h modifications upstream
2. **grpc patches**: Identify which modifications are Envoy-specific vs generally useful
3. **abseil patches**: Work with Abseil team on compatibility improvements

#### Medium-term (6 months)
1. **rules_rust patches**: Platform-specific fixes should be contributed upstream
2. **rules_foreign_cc patches**: Work with maintainers on bzlmod improvements
3. **emsdk patches**: WebAssembly toolchain improvements

#### Assessment Template
For each patched dependency:
```markdown
## Dependency: com_google_protobuf
- **Patch purpose**: Arena allocation modifications for performance
- **Upstream potential**: HIGH - performance improvements benefit entire ecosystem
- **Action**: Submit upstream PR with benchmarks
- **Timeline**: Q2 2025
- **Fallback**: Keep in extension if rejected
```

### 3. MEDIUM PRIORITY: Eliminate WORKSPACE.bzlmod

**Current State**: Minimal WORKSPACE.bzlmod files exist
**Recommendation**: Complete elimination by migrating remaining logic

```starlark
# CURRENT: WORKSPACE.bzlmod exists with minimal content
workspace(name = "envoy")

# RECOMMENDED: No WORKSPACE.bzlmod file
# All logic moved to proper bzlmod mechanisms
```

**Migration Strategy**:
1. Audit remaining WORKSPACE.bzlmod content
2. Move workspace naming to MODULE.bazel configurations
3. Migrate any remaining repository rules to extensions
4. Delete WORKSPACE.bzlmod files

### 4. MEDIUM PRIORITY: Standardize Extension Patterns

**Recommendation**: Adopt consistent extension structure across all modules

#### Standard Extension Template
```starlark
# bazel/extensions/core.bzl
def _core_impl(module_ctx):
    """Core Envoy dependencies with patches and complex setup."""
    # Group related dependencies logically
    _protobuf_setup()
    _grpc_setup()
    _boringssl_setup()
    
def _protobuf_setup():
    """Protobuf with Envoy-specific patches."""
    # Implementation with clear documentation
    
core = module_extension(
    implementation = _core_impl,
    doc = """
    Core Envoy dependencies requiring custom patches.
    
    Provides:
    - com_google_protobuf (with arena patches)
    - com_github_grpc_grpc (with Envoy modifications)
    - boringssl variants (standard and FIPS)
    """,
)
```

#### Extension Documentation Standards
Each extension should include:
- Clear purpose statement
- List of provided repositories
- Patch justification
- Migration timeline (if applicable)

### 5. LOW PRIORITY: Performance Optimizations

**Recommendation**: Leverage bzlmod-specific features for better performance

#### Conditional Loading
```starlark
# Only load expensive extensions when needed
def _should_load_mobile_deps():
    # Check if any mobile targets are being built
    return True  # Simplified logic

mobile_deps = use_extension(
    "//bazel/extensions:mobile_deps.bzl", 
    "mobile_deps",
    dev_dependency = not _should_load_mobile_deps()
)
```

#### Repository Isolation
```starlark
# Use isolated repositories for better caching
core = use_extension("//bazel/extensions:core.bzl", "core")
use_repo(core, 
    "com_google_protobuf",  # Only expose what's needed
    "com_github_grpc_grpc"
    # Don't expose internal helper repositories
)
```

## Implementation Timeline

### Phase 1: Foundation (1-2 months)
- [ ] Consolidate main module extensions (4 → 2)
- [ ] Update documentation with accurate status
- [ ] Create upstream contribution plan

### Phase 2: Streamlining (3-4 months)  
- [ ] Consolidate mobile extensions (6 → 2)
- [ ] Submit first upstream patches (protobuf, grpc)
- [ ] Eliminate WORKSPACE.bzlmod files

### Phase 3: Optimization (6-12 months)
- [ ] Migrate 5-10 dependencies to BCR as patches are accepted
- [ ] Implement performance optimizations
- [ ] Standardize extension patterns across ecosystem

### Phase 4: Ecosystem Leadership (12+ months)
- [ ] Envoy published to BCR
- [ ] Extension patterns adopted by other C++ projects
- [ ] Complete migration to upstream dependencies

## Metrics for Success

### Technical Metrics
- **Extension count**: Reduce from 12 to 6-8
- **Patched dependencies**: Reduce from 33 to <20
- **BCR adoption**: Increase from 47 to 65+ dependencies
- **Build performance**: 10-15% improvement in clean builds

### Ecosystem Metrics
- **Upstream contributions**: 10+ accepted patches per quarter
- **Community adoption**: 5+ projects using Envoy extension patterns
- **Documentation quality**: Zero outdated "COMPLETE" claims

## Risk Mitigation

### Technical Risks
1. **Upstream patch rejection**: Maintain extension fallbacks
2. **Breaking changes**: Gradual migration with rollback capability
3. **Performance regression**: Benchmark at each phase

### Process Risks
1. **Resource allocation**: Assign dedicated maintainer for bzlmod improvements
2. **Community coordination**: Regular sync with BCR maintainers
3. **Timeline pressure**: Prioritize high-impact, low-risk changes first

## Conclusion

Envoy's bzlmod implementation is well-positioned for success. By consolidating extensions, contributing patches upstream, and eliminating remaining WORKSPACE dependencies, Envoy can become a model for large C++ project migration to bzlmod.

The recommended changes will:
- **Reduce complexity** through extension consolidation
- **Improve ecosystem health** via upstream contributions  
- **Enhance performance** through bzlmod-specific optimizations
- **Simplify maintenance** with standardized patterns

This positions Envoy as a leader in Bazel 8.0+ adoption and provides clear value to both the Envoy project and the broader Bazel ecosystem.
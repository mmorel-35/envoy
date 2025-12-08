# Bzlmod Migration Guide for Envoy

This document provides practical guidance for continuing Envoy's migration to Bazel's new module system (bzlmod).

## Current Status

Envoy has begun migrating from WORKSPACE to bzlmod. Both build systems currently work:

- **WORKSPACE mode** (legacy): `bazel build --noenable_bzlmod //...`
- **Bzlmod mode** (new): `bazel build --enable_bzlmod //...`

## Architecture

### Module Extensions

Envoy uses module extensions to load dependencies not yet in Bazel Central Registry (BCR):

- **`bazel/extensions.bzl`**: Main Envoy runtime dependencies (~75 non-BCR repos)
- **`bazel/extensions.bzl`**: Envoy development dependencies (testing, linting tools)
- **`api/bazel/extensions.bzl`**: Envoy API dependencies
- **`mobile/bazel/extensions.bzl`**: Envoy Mobile dependencies

### Separating Dev Dependencies

Dependencies are separated into runtime and development:

```python
# In MODULE.bazel
# Runtime dependencies
envoy_deps = use_extension("//bazel:extensions.bzl", "envoy_dependencies_extension")
use_repo(envoy_deps, "boringssl_fips", ...)

# Development dependencies (testing, linting)
envoy_dev_deps = use_extension(
    "//bazel:extensions.bzl",
    "envoy_dev_dependencies_extension",
    dev_dependency = True,  # Won't be loaded by dependents
)
use_repo(envoy_dev_deps, "com_github_bazelbuild_buildtools", ...)
```

### Using git_override for Unpublished Modules

For dependencies with MODULE.bazel that need specific commits:

```python
bazel_dep(name = "toolchains_llvm", version = "1.0.0")

git_override(
    module_name = "toolchains_llvm",
    commit = "fb29f3d53757790dad17b90df0794cea41f1e183",
    remote = "https://github.com/bazel-contrib/toolchains_llvm",
)
```

This allows using dependencies that:
- Have MODULE.bazel but aren't in BCR yet
- Need a specific commit with patches
- Require unreleased features

### Pattern: Reuse with Conditionals

All extensions follow the same pattern to avoid code duplication:

```python
# In bazel/repositories.bzl
def envoy_dependencies(skip_targets = [], bzlmod = False):
    """Load dependencies for both WORKSPACE and bzlmod modes."""
    
    # BCR dependencies - skip in bzlmod mode
    if not bzlmod:
        external_http_archive("zlib")  # In BCR
        _com_google_protobuf()  # In BCR
    
    # Non-BCR dependencies - always load
    _boringssl_fips()  # Not in BCR
    _com_github_grpc_grpc(bzlmod=bzlmod)  # Has patches

# In bazel/extensions.bzl
def _envoy_dependencies_impl(module_ctx):
    envoy_dependencies(bzlmod = True)  # Skips BCR deps

envoy_dependencies_extension = module_extension(
    implementation = _envoy_dependencies_impl,
)
```

## Adding New Dependencies

### If Dependency is in BCR

Add to `MODULE.bazel`:

```python
bazel_dep(name = "dependency_name", version = "1.0.0")
```

Wrap existing WORKSPACE loading with conditional:

```python
# In bazel/repositories.bzl
def envoy_dependencies(bzlmod = False):
    if not bzlmod:
        _dependency_name()  # Only load in WORKSPACE mode
```

### If Dependency is NOT in BCR

Add to the repository function:

```python
# In bazel/repositories.bzl
def envoy_dependencies(bzlmod = False):
    _new_dependency()  # Load for both modes
```

Register in MODULE.bazel:

```python
envoy_deps = use_extension("//bazel:extensions.bzl", "envoy_dependencies_extension")
use_repo(
    envoy_deps,
    "new_dependency",
)
```

## Handling Patches

Patches are preserved across both modes:

```python
def _com_github_grpc_grpc(bzlmod = False):
    grpc_kwargs = {
        "name": "com_github_grpc_grpc",
        "patches": ["@envoy//bazel:grpc.patch"],  # Same patches
    }
    
    # Skip repo_mapping in bzlmod (not supported)
    if not bzlmod:
        grpc_kwargs["repo_mapping"] = {"@openssl": "@boringssl"}
    
    external_http_archive(**grpc_kwargs)
```

## Bzlmod-Specific Limitations

### No `repo_mapping`

Bzlmod handles repository mapping automatically. Remove `repo_mapping` parameter in bzlmod mode:

```python
def _dependency(bzlmod = False):
    kwargs = {"name": "dep"}
    if not bzlmod:
        kwargs["repo_mapping"] = {"@old": "@new"}
    external_http_archive(**kwargs)
```

### No `native.new_local_repository()`

Cannot create local repositories in module extensions. Skip or use alternatives:

```python
def _com_google_cel_cpp(bzlmod = False):
    external_http_archive(name = "com_google_cel_cpp")
    
    if not bzlmod:
        # This only works in WORKSPACE mode
        native.new_local_repository(...)
```

## Testing Both Modes

Always test changes in both modes:

```bash
# WORKSPACE mode
bazel build --noenable_bzlmod //source/exe:envoy-static
bazel test --noenable_bzlmod //test/...

# Bzlmod mode
bazel build --enable_bzlmod //source/exe:envoy-static
bazel test --enable_bzlmod //test/...

# Verify module graph
bazel mod graph --enable_bzlmod
```

## Migration Checklist

When migrating a dependency to BCR:

- [ ] Verify dependency is published to BCR
- [ ] Add `bazel_dep()` to MODULE.bazel
- [ ] Wrap existing load with `if not bzlmod:` in repositories.bzl
- [ ] Remove from `use_repo()` in extensions.bzl
- [ ] Test both WORKSPACE and bzlmod modes
- [ ] Update this document if needed

## Common Issues

### Dependency loaded twice

**Symptom**: Error about repository already existing

**Fix**: Wrap the dependency with `if not bzlmod:` check

### Missing repository in bzlmod

**Symptom**: `No such repository` error in bzlmod mode

**Fix**: Add to `use_repo()` in MODULE.bazel

### Patches not applying

**Symptom**: Build fails with patch errors

**Fix**: Ensure patch paths use `@envoy//` or `@envoy_mobile//` prefix

## References

- [Official Bazel Bzlmod Guide](https://bazel.build/external/migration)
- [Bazel Central Registry](https://registry.bazel.build/)
- [Module Extensions Documentation](https://bazel.build/external/extension)

## Getting Help

- Check existing module extensions: `bazel/extensions.bzl`, `api/bazel/extensions.bzl`
- Review BCR availability: https://registry.bazel.build/
- Ask in Envoy Slack #build channel

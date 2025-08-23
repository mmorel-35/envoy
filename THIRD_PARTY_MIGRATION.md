# WORKSPACE to Bzlmod Migration with third_party Compatibility Layer

This document describes the incremental migration path from WORKSPACE + bind() + //external: references to bzlmod-native dependencies.

## Migration Strategy

### 1. Compatibility Layer (third_party/)

We've created a `third_party/BUILD.bazel` file that provides aliases for all legacy `//external:foo` references:

```starlark
alias(
    name = "ssl",
    actual = "@envoy//bazel:boringssl",
)

alias(
    name = "protobuf",
    actual = "@com_google_protobuf//:protobuf", 
)
```

### 2. Redirect //external: to //third_party:

The `envoy_external_dep_path()` function in `bazel/envoy_internal.bzl` now returns `//third_party:foo` instead of `//external:foo`:

```starlark
def envoy_external_dep_path(dep):
    return "//third_party:%s" % dep
```

This automatically migrates all usage through Envoy's build macros.

### 3. Updated bind() References

Legacy bind() calls that created circular references have been updated:

```starlark
# Before
native.bind(name = "libssl", actual = "//external:ssl")

# After  
native.bind(name = "libssl", actual = "//third_party:ssl")
```

### 4. External BUILD File Updates

External dependency BUILD files have been updated to use the compatibility layer:

```starlark
# bazel/external/libprotobuf_mutator.BUILD
deps = ["//third_party:protobuf"]  # was "//external:protobuf"
```

## New Code Guidelines

**For new code, avoid both `//external:` and `//third_party:` references.**

Instead, depend directly on the bzlmod repository:

```starlark
# Good - direct bzlmod dependency
deps = ["@com_google_protobuf//:protobuf"]

# Avoid - compatibility layer (temporary)
deps = ["//third_party:protobuf"] 

# Avoid - legacy external (deprecated)
deps = ["//external:protobuf"]
```

## Migration Path Forward

1. **Phase 1** ✅ - Create compatibility layer and redirect references
2. **Phase 2** - Gradually migrate direct //third_party: references to @repo//:target  
3. **Phase 3** - Handle third-party dependencies that use //external: with patches
4. **Phase 4** - Remove third_party/ compatibility layer once all references are cleaned up

## Benefits

- **Incremental migration** - No breaking changes during transition
- **Clear separation** - third_party/ is clearly marked as temporary compatibility
- **Future-ready** - Direct @repo//:target dependencies are bzlmod-native
- **Maintainable** - Single source of truth for dependency mappings

## Current Status

✅ third_party/ compatibility layer created  
✅ //external: references redirected to //third_party:  
✅ Circular bind() references resolved  
✅ External BUILD files updated  
🔄 Ready for gradual migration to direct @repo//:target usage
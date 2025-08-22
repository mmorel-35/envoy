# Bazel WORKSPACE to bzlmod Migration Strategy

This document outlines the progressive migration strategy from Bazel WORKSPACE to bzlmod for the Envoy project.

## Current State

- **WORKSPACE**: Primary dependency management with 34 dependencies using patches
- **MODULE.bazel**: Partially implemented with bzlmod disabled via `.bazelrc`
- **Patches**: Critical for many dependencies and must be preserved during migration

## Migration Strategy

### Phase 1: Hybrid Setup (Current)
- Keep bzlmod disabled by default (`--noenable_bzlmod`)
- Use WORKSPACE for all dependencies requiring patches
- Use MODULE.bazel only for dependencies that don't need patches
- Use `non_module_dependencies` extension for patch-requiring dependencies

### Phase 2: Conditional Migration (Next)
- Enable bzlmod conditionally via build flag
- Create mechanism to identify which dependencies can safely migrate
- Maintain patch application for dependencies that need them

### Phase 3: Progressive Migration (Future)
- Migrate dependencies one-by-one as patches are upstreamed
- Track migration progress in this document
- Eventually phase out WORKSPACE entirely

## Dependencies Requiring Patches

The following dependencies currently require patches and MUST remain in WORKSPACE until patches are upstreamed:

### Core Dependencies with Patches
- `protobuf` - Has protobuf.patch
- `grpc` - Has grpc.patch  
- `googletest` - Has googletest.patch
- `rules_rust` - Has rules_rust.patch + rules_rust_ppc64le.patch
- `rules_fuzzing` - Has rules_fuzzing.patch
- `rules_foreign_cc` - Has rules_foreign_cc.patch
- `abseil-cpp` - Has abseil.patch
- `c-ares` - Has c-ares.patch

### Foreign CC Dependencies with Patches
- `com_github_unicode_org_icu` - foreign_cc:icu.patch
- `nghttp2` - foreign_cc:nghttp2.patch
- `zlib` - foreign_cc:zlib.patch
- `cel-cpp` - foreign_cc:cel-cpp.patch
- `hyperscan` - foreign_cc:hyperscan.patch
- `vectorscan` - foreign_cc:vectorscan.patch
- `luajit` - foreign_cc:luajit.patch
- `librdkafka` - foreign_cc:librdkafka.patch
- `vpp_vcl` - foreign_cc:vpp_vcl.patch
- `dlb` - foreign_cc:dlb.patch
- `qatzstd` - foreign_cc:qatzstd.patch

### Platform-Specific and Tool Dependencies
- `boringssl` - boringssl_fips.patch
- `tcmalloc` - tcmalloc.patch
- `emsdk` - emsdk.patch
- `rules_apple` - rules_apple.patch
- `aspect_bazel_lib` - aspect.patch

### Extension-Specific Dependencies
- `com_google_protoconverter` - com_google_protoconverter.patch
- `com_google_protoprocessinglib` - proto_processing_lib.patch
- `com_github_skyapm_cpp2sky` - com_github_skyapm_cpp2sky.patch
- `skywalking_data_collect_protocol` - skywalking_data_collect_protocol.patch
- `googleurl` - external:googleurl.patch

## Current Status

### Dependencies Moved to WORKSPACE-Only (Require Patches)

The following dependencies have been temporarily removed from MODULE.bazel because they require patches:

- `abseil-cpp` - Has abseil.patch
- `c-ares` - Has c-ares.patch  
- `grpc` - Has grpc.patch
- `googletest` - Has googletest.patch
- `protobuf` - Has protobuf.patch
- `rules_foreign_cc` - Has rules_foreign_cc.patch
- `rules_fuzzing` - Has rules_fuzzing.patch
- `rules_rust` - Has rules_rust.patch + rules_rust_ppc64le.patch

These are handled via the `non_module_dependencies` extension when bzlmod is enabled.

### Dependencies Safe for bzlmod

These dependencies are already in MODULE.bazel and don't require patches:

- `bazel_skylib`
- `bazel_features`
- `platforms`
- `rules_cc`
- `rules_pkg`
- `rules_python`
- `rules_shell`
- `rules_proto`
- `rules_java`
- `fmt`
- `spdlog`
- `tclap`
- `xxhash`
- `yaml-cpp`
- `xds`
- `libcircllhist`
- `protoc-gen-validate`
- `googleapis`
- `google_benchmark`
- `re2` (dev dependency)

## Validation and Testing

### Using the Validation Script

A validation script is provided to help test the hybrid setup:

```bash
./scripts/bzlmod_validation.sh
```

This script:
- Tests WORKSPACE build queries  
- Tests bzlmod build queries
- Checks for dependency conflicts
- Validates patch files exist
- Provides guidance on next steps

### Manual Testing

1. **Test WORKSPACE mode (default)**:
   ```bash
   bazel build //source/exe:envoy-static
   ```

2. **Test bzlmod mode (experimental)**:
   ```bash
   bazel build --config=bzlmod //source/exe:envoy-static
   ```

3. **Compare outputs** to ensure both modes work equivalently

## Implementation Guidelines

### For Maintainers

1. **Before migrating a dependency to bzlmod:**
   - Verify it doesn't require patches in `bazel/repositories.bzl`
   - Check if patches can be upstreamed to eliminate need
   - Test that the bzlmod version provides equivalent functionality

2. **When adding new dependencies:**
   - Prefer bzlmod if no patches are needed
   - Use WORKSPACE if patches are required
   - Document patch rationale per DEPENDENCY_POLICY.md

3. **When updating existing dependencies:**
   - Check if patches can be removed (upstreamed fixes)
   - Consider migration to bzlmod if patches no longer needed

### Build Configuration

- **Default**: `bazel build <target>` - bzlmod disabled, use WORKSPACE
- **Experimental**: `bazel build --config=bzlmod <target>` - hybrid bzlmod + WORKSPACE
- **Future**: bzlmod enabled by default when most patches are eliminated

The experimental bzlmod configuration allows testing the hybrid setup while keeping WORKSPACE as the stable default.

## Progress Tracking

### Phase 1: Infrastructure Setup ✅
- [x] Document current state and strategy
- [x] Identify all dependencies with patches  
- [x] Categorize safe vs. unsafe dependencies
- [x] Remove conflicting dependencies from MODULE.bazel
- [x] Add bzlmod build configuration flag
- [x] Update non_module_dependencies use_repo list
- [ ] Test hybrid setup

### Phase 2: Validation and Testing (Current)
- [ ] Test build with WORKSPACE (default)
- [ ] Test build with bzlmod flag  
- [ ] Verify patched dependencies work correctly
- [ ] Validate that both modes produce equivalent builds

### Phase 3: Full Migration (Future)
- [ ] Enable bzlmod by default
- [ ] Phase out WORKSPACE
- [ ] Remove legacy compatibility code

## References

- [Bazel bzlmod documentation](https://bazel.build/external/module)
- [Envoy Dependency Policy](DEPENDENCY_POLICY.md)
- [Repository Locations](bazel/repository_locations.bzl)
- [Patches Directory](bazel/)
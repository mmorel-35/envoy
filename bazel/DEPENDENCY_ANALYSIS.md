# Bzlmod Dependency Migration Analysis

This document provides a comprehensive analysis of Envoy's dependencies for potential migration from module extensions to BCR (Bazel Central Registry) or git_override.

## Overview

As of the current migration state:
- **49 dependencies** loaded via `bazel_dep` from BCR
- **77 dependencies** loaded via `use_repo(envoy_deps, ...)` from module extension
- **20 dependencies** use `repo_name` mapping

## Dependency Categories

### ⚙️ Tool Binaries (11)

These are platform-specific binaries used during build. **Keep as-is** in module extension.

**Protoc binaries** (6):
- `com_google_protobuf_protoc_linux_aarch_64`
- `com_google_protobuf_protoc_linux_x86_64`
- `com_google_protobuf_protoc_linux_ppcle_64`
- `com_google_protobuf_protoc_osx_aarch_64`
- `com_google_protobuf_protoc_osx_x86_64`
- `com_google_protobuf_protoc_win64`

**FIPS build tools** (5):
- `fips_ninja`
- `fips_cmake_linux_x86_64`
- `fips_cmake_linux_aarch64`
- `fips_go_linux_amd64`
- `fips_go_linux_arm64`

**Recommendation**: Keep in module extension. These are build tools, not libraries.

---

### 🔗 git_override Candidates (5)

Dependencies that have MODULE.bazel in their repositories. Can potentially use `git_override` pattern (like `toolchains_llvm`).

1. **build_bazel_rules_apple**
   - Repository: https://github.com/bazelbuild/rules_apple
   - BCR name: `rules_apple` (available in BCR)
   - Status: ✅ Could use bazel_dep + git_override for specific commit
   - Note: Currently has patches applied

2. **com_github_grpc_grpc**
   - Repository: https://github.com/grpc/grpc
   - BCR name: `grpc` (checking availability)
   - Status: ⚠️ Could use git_override if in BCR or has MODULE.bazel
   - Note: Has patches and custom repo_mapping

3. **com_google_cel_cpp**
   - Repository: https://github.com/google/cel-cpp
   - Status: ⚠️ Has MODULE.bazel, could use git_override
   - Note: Has patches and uses native.new_local_repository (not compatible with bzlmod)

4. **io_opentelemetry_cpp**
   - Repository: https://github.com/open-telemetry/opentelemetry-cpp
   - BCR name: Potentially available
   - Status: ⚠️ Check if in BCR or has MODULE.bazel
   - Note: May need specific version

5. **rules_proto_grpc**
   - Repository: https://github.com/rules-proto-grpc/rules_proto_grpc
   - Status: ⚠️ Has MODULE.bazel
   - Note: gRPC-related rules

**Recommendation**: Investigate each for:
- Availability in BCR
- MODULE.bazel version compatibility
- Patch compatibility with git_override

---

### 🛠️ Custom Build Files (61)

Dependencies requiring custom BUILD files, patches, or not yet in BCR. **Keep in module extension** for now.

#### SSL/TLS Libraries (2)
- `boringssl_fips` - FIPS variant not in BCR
- `aws_lc` - AWS's BoringSSL fork

#### C++ Dependencies (38)
- `grpc_httpjson_transcoding`
- `com_google_protoconverter`
- `com_google_protofieldextraction`
- `com_google_protoprocessinglib`
- `ocp`
- `com_github_openhistogram_libcircllhist`
- `com_github_awslabs_aws_c_auth`
- `com_github_axboe_liburing`
- `com_github_c_ares_c_ares` (c-ares might be in BCR as `c-ares`)
- `com_github_envoyproxy_sqlparser`
- `com_github_mirror_tclap`
- `com_github_google_libprotobuf_mutator`
- `com_github_google_libsxg`
- `com_github_unicode_org_icu`
- `com_github_intel_ipp_crypto_crypto_mb`
- `com_github_intel_qatlib`
- `com_github_intel_qatzip`
- `com_github_qat_zstd`
- `com_github_lz4_lz4`
- `com_github_libevent_libevent` (libevent might be in BCR)
- `net_colm_open_source_colm`
- `net_colm_open_source_ragel`
- `com_github_zlib_ng_zlib_ng`
- `org_boost`
- `com_github_luajit_luajit`
- `com_github_nghttp2_nghttp2`
- `com_github_msgpack_cpp`
- `com_github_ncopa_suexec`
- `com_github_maxmind_libmaxminddb`
- `com_github_google_tcmalloc`
- `com_github_google_perfetto`
- `com_github_datadog_dd_trace_cpp`
- `com_github_skyapm_cpp2sky`
- `com_github_alibaba_hessian2_codec`
- `com_github_fdio_vpp_vcl`
- `intel_dlb`
- `org_llvm_releases_compiler_rt`
- `com_github_google_jwt_verify`

#### V8 and Related (5)
- `v8`
- `dragonbox`
- `fp16`
- `simdutf`
- `intel_ittapi`

#### QUICHE and URL (2)
- `com_github_google_quiche`
- `googleurl`

#### Proxy WASM (3)
- `proxy_wasm_cpp_sdk`
- `proxy_wasm_cpp_host`
- `proxy_wasm_rust_sdk`

#### Regex Engines (2)
- `io_hyperscan`
- `io_vectorscan`

#### Build Tools (4)
- `bazel_toolchains`
- `bazel_compdb`
- `envoy_examples`
- `envoy_toolshed`

#### Kafka (3)
- `kafka_source`
- `confluentinc_librdkafka`
- `kafka_server_binary`

#### WASM Runtimes (2)
- `com_github_wamr`
- `com_github_wasmtime`

**Recommendation**:
- Check if any are now available in BCR (e.g., c-ares, libevent)
- Keep others in module extension until BCR support available
- Monitor BCR for new additions

---

## Migration Checklist

When a dependency becomes available in BCR:

- [ ] Verify the BCR version meets Envoy's requirements
- [ ] Test that patches are compatible (or contributed to BCR)
- [ ] Add `bazel_dep(name = "...", version = "...")` to MODULE.bazel
- [ ] Wrap existing load with `if not bzlmod:` in repositories.bzl
- [ ] Remove from `use_repo()` in MODULE.bazel
- [ ] Update extension comments in bazel/extensions.bzl
- [ ] Test both WORKSPACE and bzlmod modes
- [ ] Update this document

## git_override Pattern

For dependencies with MODULE.bazel that need specific commits:

```python
# In MODULE.bazel
bazel_dep(name = "dependency_name", version = "1.0.0")

git_override(
    module_name = "dependency_name",
    commit = "abc123...",
    remote = "https://github.com/org/repo",
)
```

This pattern:
- Uses dependencies with MODULE.bazel not yet in BCR
- Allows pinning to specific commits
- Supports unreleased features
- Works with patches if needed

**Current examples**: `toolchains_llvm`, `xds`

---

## Status Summary

| Category | Count | Migration Status |
|----------|-------|------------------|
| Already in BCR (bazel_dep) | 49 | ✅ Migrated |
| Tool binaries | 11 | 🔒 Keep in extension |
| git_override candidates | 5 | 🔄 Needs investigation |
| Custom build files | 61 | ⏳ Monitor BCR availability |
| **Total non-BCR deps** | **77** | |

## Next Steps

1. **Immediate**: Verify git_override candidates have MODULE.bazel
2. **Short term**: Check BCR for newly added packages (c-ares, libevent, etc.)
3. **Medium term**: Monitor upstream projects for MODULE.bazel adoption
4. **Long term**: Contribute MODULE.bazel to upstream projects where missing

## References

- [Bazel Central Registry](https://registry.bazel.build/)
- [Bazel Module Migration Guide](https://bazel.build/external/migration)
- [Module Extension Documentation](https://bazel.build/external/extension)

---

**Last Updated**: Analysis based on current MODULE.bazel and repository state
**Maintained by**: Envoy build team

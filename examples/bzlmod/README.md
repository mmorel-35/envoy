# Bzlmod Migration Example

This directory contains examples and documentation for working with Envoy's bzlmod migration.

## Quick Start

After the migration, you can use the following commands to explore the new dependency management:

### View Module Dependencies
```bash
# Show all external repositories managed by bzlmod
bazel mod show_extension_repos

# Display the full dependency graph
bazel mod graph
```

### Validate Migration
```bash
# Run the validation script
./tools/bzlmod_validate.sh
```

### Test Core Builds
```bash
# Test basic library builds
bazel build //source/common/common:version_lib
bazel build //source/common/protobuf:utility_lib

# Test analysis phase for larger targets
bazel query "deps(//source/exe:envoy-static)" --output=label_kind
```

## Migration Examples

### Adding a New Bzlmod Dependency

If you want to migrate an additional dependency from WORKSPACE to MODULE.bazel:

1. **Check if it's available in BCR**: Visit https://registry.bazel.build/
2. **Add to MODULE.bazel**:
   ```starlark
   bazel_dep(name = "module_name", version = "x.y.z", repo_name = "original_repo_name")
   ```
3. **Remove from WORKSPACE**: Comment out or remove the corresponding `http_archive` or similar rule
4. **Test**: Ensure your build still works

### Example: Migrating re2
```starlark
# In MODULE.bazel
bazel_dep(name = "re2", version = "2024.12.01", repo_name = "com_googlesource_code_re2")
```

### Example: Migrating zlib
```starlark
# In MODULE.bazel  
bazel_dep(name = "zlib", version = "1.3.1", repo_name = "net_zlib")
```

## Troubleshooting

### Common Issues

1. **Repository name conflicts**: Use `repo_name` parameter to maintain compatibility
2. **Version conflicts**: Bzlmod will automatically resolve to the highest compatible version
3. **Missing modules**: Check BCR or create a custom module extension

### Debugging Commands
```bash
# Check what repositories are available
bazel query "//external:*" --output=package

# Debug module resolution
bazel mod explain //external:some_repo

# View module resolution trace
bazel mod show_extension --debug
```

## Migration Checklist

- [ ] Core C++ libraries (protobuf, abseil, googletest) ✅ DONE
- [ ] Build rules (rules_cc, rules_proto, etc.) ✅ DONE
- [ ] gRPC and related dependencies
- [ ] Platform-specific rules (rules_apple, etc.)
- [ ] Envoy-specific repositories (envoy_api, etc.)
- [ ] Extension and contrib dependencies

## Resources

- [Bazel Migration Guide](https://bazel.build/external/migration)
- [Bazel Central Registry](https://registry.bazel.build/)
- [MODULE.bazel Reference](https://bazel.build/external/mod)
- [Envoy Migration Documentation](../docs/root/start/migrating/bzlmod.md)
# Bzlmod Migration Quick Start

This guide helps you use the new hybrid WORKSPACE + bzlmod setup in Envoy.

## TL;DR

- **Default builds**: `bazel build <target>` (uses WORKSPACE, stable)
- **Experimental builds**: `bazel build --config=bzlmod <target>` (hybrid mode)
- **Validation**: `./scripts/bzlmod_validation.sh`

## What Changed

Envoy now supports a **hybrid migration approach** from WORKSPACE to bzlmod:

1. **Dependencies with patches** remain in WORKSPACE (34 dependencies)
2. **Dependencies without patches** can use bzlmod (20+ dependencies) 
3. **Both modes work** - you can test and compare

## Common Commands

### Standard Build (WORKSPACE only)
```bash
bazel build //source/exe:envoy-static
bazel test //test/...
```

### Experimental Build (Hybrid bzlmod)
```bash
bazel build --config=bzlmod //source/exe:envoy-static
bazel test --config=bzlmod //test/...
```

### Validate Setup
```bash
./scripts/bzlmod_validation.sh
```

## Why This Approach?

- **Minimal disruption**: WORKSPACE remains the default and stable option
- **Patch preservation**: Critical patches continue to work
- **Progressive migration**: Dependencies can migrate individually as patches are upstreamed
- **Testing friendly**: Both modes can be compared side-by-side

## For Maintainers

- **Adding dependencies**: Use bzlmod if no patches needed, WORKSPACE if patches required
- **Updating dependencies**: Check if patches can be removed to enable bzlmod migration
- **Migration progress**: Track in [BZLMOD_MIGRATION.md](BZLMOD_MIGRATION.md)

## Troubleshooting

### Build fails with bzlmod
Try the WORKSPACE version first to isolate issues:
```bash
bazel build <target>                    # WORKSPACE mode
bazel build --config=bzlmod <target>    # bzlmod mode
```

### Dependency conflicts
Run the validation script:
```bash
./scripts/bzlmod_validation.sh
```

### Want to migrate a dependency?
1. Check if it has patches in `bazel/repositories.bzl`
2. If no patches: add to `MODULE.bazel`
3. If patches exist: upstream them first, then migrate

## More Information

- [Complete Migration Strategy](BZLMOD_MIGRATION.md)
- [Dependency Policy](DEPENDENCY_POLICY.md)
- [Bazel bzlmod docs](https://bazel.build/external/module)
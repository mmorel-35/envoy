# Bazel Central Registry (BCR) Publishing Guide

This directory contains templates and documentation for publishing Envoy to the Bazel Central Registry (BCR).

## Overview

The Bazel Central Registry (BCR) is the default registry for Bazel modules using bzlmod. Publishing Envoy to BCR makes it easy for other projects to depend on Envoy using simple `bazel_dep()` declarations.

## Prerequisites

- A released version of Envoy with a git tag (e.g., `v1.29.0`)
- The release archive published on GitHub releases
- Access to fork and create PRs in [bazel-central-registry](https://github.com/bazelbuild/bazel-central-registry)

## Publishing Process

### 1. Prepare Release Information

When a new version of Envoy is released, gather the following information:

- **Version**: The version number (e.g., `1.29.0`)
- **Release Date**: The date the release was published
- **Tag**: The git tag for the release (e.g., `v1.29.0`)
- **SHA256**: The SHA256 hash of the release archive

To calculate the SHA256:

```bash
VERSION="1.29.0"
wget https://github.com/envoyproxy/envoy/archive/refs/tags/v${VERSION}.tar.gz
sha256sum v${VERSION}.tar.gz
```

### 2. Fork the Bazel Central Registry

1. Fork the [bazel-central-registry](https://github.com/bazelbuild/bazel-central-registry) repository
2. Clone your fork locally

```bash
git clone https://github.com/YOUR_USERNAME/bazel-central-registry.git
cd bazel-central-registry
```

### 3. Create Module Directory

Create a directory for the new Envoy version:

```bash
VERSION="1.29.0"
mkdir -p modules/envoy/${VERSION}
```

### 4. Copy MODULE.bazel

Copy the MODULE.bazel from the release to the BCR:

```bash
# From your Envoy repository
cp MODULE.bazel /path/to/bazel-central-registry/modules/envoy/${VERSION}/
```

**Important**: Review the MODULE.bazel to ensure:
- All `local_path_override` declarations are removed
- All `git_override` declarations are removed or converted to `bazel_dep` with proper versions
- The module version matches the release version
- All dependencies are available in BCR or properly documented

### 5. Create source.json

Copy and fill in the source.json template:

```bash
cp /path/to/envoy/.bcr/source.template.json \
   /path/to/bazel-central-registry/modules/envoy/${VERSION}/source.json
```

Edit the file to replace:
- `{version}` with the actual version
- `REPLACE_WITH_ACTUAL_SHA256_OF_RELEASE_ARCHIVE` with the actual SHA256 hash

Example:

```json
{
  "integrity": "sha256-abc123...",
  "strip_prefix": "envoy-1.29.0",
  "url": "https://github.com/envoyproxy/envoy/archive/refs/tags/v1.29.0.tar.gz"
}
```

### 6. Update metadata.json

If this is the first time publishing Envoy to BCR:

```bash
cp /path/to/envoy/.bcr/metadata.template.json \
   /path/to/bazel-central-registry/modules/envoy/metadata.json
```

If Envoy is already in BCR, update the existing metadata.json:

```bash
cd /path/to/bazel-central-registry/modules/envoy
# Add the new version to the "versions" array in metadata.json
```

Example metadata.json after adding version 1.29.0:

```json
{
  "homepage": "https://github.com/envoyproxy/envoy",
  "maintainers": [
    {
      "email": "envoy-maintainers@googlegroups.com",
      "github": "envoyproxy",
      "name": "Envoy Maintainers"
    }
  ],
  "repository": [
    "github:envoyproxy/envoy"
  ],
  "versions": ["1.28.0", "1.29.0"],
  "yanked_versions": {}
}
```

### 7. Add Presubmit Configuration (Optional)

If you want to add BCR-specific tests:

```bash
cp /path/to/envoy/.bcr/presubmit.template.yml \
   /path/to/bazel-central-registry/modules/envoy/${VERSION}/presubmit.yml
```

Customize the test targets as needed.

### 8. Test Locally

Before submitting, test the module locally:

```bash
cd /path/to/bazel-central-registry

# Test building the module
bazelisk test //modules/envoy/${VERSION}/...

# Or use the BCR validation tool
./tools/bcr_validation.sh modules/envoy/${VERSION}
```

### 9. Create Pull Request

1. Commit your changes:

```bash
git add modules/envoy/${VERSION}
git commit -m "Add Envoy ${VERSION} to BCR"
git push origin main
```

2. Create a pull request to [bazel-central-registry](https://github.com/bazelbuild/bazel-central-registry)

3. Wait for CI checks to pass and for review from BCR maintainers

### 10. Announce Release

Once the PR is merged, announce the new BCR release:

1. Update the Envoy documentation to mention BCR availability
2. Post on the Envoy mailing list
3. Update any migration guides

## Using Envoy from BCR

Once published, users can depend on Envoy using:

```starlark
# In their MODULE.bazel
bazel_dep(name = "envoy", version = "1.29.0")
```

## API Module

The Envoy API can also be published separately to BCR as `envoy_api`. Follow the same process but:

1. Use the `api/` directory as the module root
2. Use the `api/MODULE.bazel` file
3. Create the module under `modules/envoy_api/` in BCR
4. Use appropriate strip_prefix in source.json: `envoy-{version}/api`

## Troubleshooting

### Module Dependencies Not Found

If a dependency is not available in BCR:
1. Check if it's available under a different name
2. Consider publishing the dependency first
3. Use a module extension for non-BCR dependencies (documented in the MODULE.bazel)

### Build Failures in CI

1. Check that all overrides are removed from MODULE.bazel
2. Verify all dependencies are available in BCR
3. Test locally with a clean Bazel cache
4. Check the BCR CI logs for specific errors

### Version Conflicts

If there are version conflicts with dependencies:
1. Try to use compatible versions that are in BCR
2. Document any version constraints in the MODULE.bazel
3. Consider using version ranges where appropriate

## References

- [Bazel Central Registry](https://registry.bazel.build/)
- [BCR Contribution Guide](https://github.com/bazelbuild/bazel-central-registry/blob/main/docs/README.md)
- [Bzlmod User Guide](https://bazel.build/external/overview#bzlmod)
- [Envoy Bzlmod Migration Guide](../BZLMOD_MIGRATION_GUIDE.md)

## Support

For questions about BCR publishing:
- Open an issue in the [bazel-central-registry](https://github.com/bazelbuild/bazel-central-registry/issues) repository
- Ask on the [Bazel Slack](https://slack.bazel.build/) in #bzlmod channel
- Contact Envoy maintainers at envoy-maintainers@googlegroups.com

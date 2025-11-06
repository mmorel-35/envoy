#!/bin/bash
# Script to help prepare a BCR submission for Envoy
# Usage: ./prepare-bcr-submission.sh <version> <bcr-repo-path>

set -e

VERSION="${1}"
BCR_PATH="${2}"

if [ -z "$VERSION" ] || [ -z "$BCR_PATH" ]; then
    echo "Usage: $0 <version> <bcr-repo-path>"
    echo "Example: $0 1.29.0 /path/to/bazel-central-registry"
    exit 1
fi

# Remove 'v' prefix if present
VERSION="${VERSION#v}"

echo "Preparing BCR submission for Envoy version $VERSION"
echo "BCR repository path: $BCR_PATH"

# Validate BCR path
if [ ! -d "$BCR_PATH" ]; then
    echo "Error: BCR repository path does not exist: $BCR_PATH"
    exit 1
fi

if [ ! -d "$BCR_PATH/modules" ]; then
    echo "Error: $BCR_PATH does not appear to be a BCR repository (missing modules/ directory)"
    exit 1
fi

# Create module directory
MODULE_DIR="$BCR_PATH/modules/envoy/$VERSION"
echo "Creating module directory: $MODULE_DIR"
mkdir -p "$MODULE_DIR"

# Get the repository root (parent of .bcr directory)
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Check if we're in a git repository and if the tag exists
if ! git -C "$REPO_ROOT" rev-parse "v$VERSION" >/dev/null 2>&1; then
    echo "Warning: Git tag v$VERSION not found in repository"
    echo "Make sure you're running this on the correct git tag"
fi

# Copy MODULE.bazel
echo "Copying MODULE.bazel..."
cp "$REPO_ROOT/MODULE.bazel" "$MODULE_DIR/MODULE.bazel"

# Check for and warn about overrides in MODULE.bazel
echo "Checking MODULE.bazel for overrides..."
if grep -q "override\|local_path" "$MODULE_DIR/MODULE.bazel"; then
    echo ""
    echo "WARNING: MODULE.bazel contains overrides that need to be removed:"
    grep -n "override\|local_path" "$MODULE_DIR/MODULE.bazel" || true
    echo ""
    echo "Please edit $MODULE_DIR/MODULE.bazel to remove these overrides"
    echo "Press Enter to continue after editing, or Ctrl+C to abort"
    read -r
fi

# Update version in MODULE.bazel
echo "Updating version in MODULE.bazel to $VERSION..."
sed -i "s/version = \".*\"/version = \"$VERSION\"/" "$MODULE_DIR/MODULE.bazel"

# Download release archive and calculate SHA256
ARCHIVE_URL="https://github.com/envoyproxy/envoy/archive/refs/tags/v$VERSION.tar.gz"
echo "Downloading release archive from $ARCHIVE_URL"
TEMP_ARCHIVE=$(mktemp)
trap "rm -f $TEMP_ARCHIVE" EXIT

if ! curl -L -o "$TEMP_ARCHIVE" "$ARCHIVE_URL"; then
    echo "Error: Failed to download release archive"
    echo "Make sure version $VERSION is released on GitHub"
    exit 1
fi

echo "Calculating SHA256..."
SHA256=$(sha256sum "$TEMP_ARCHIVE" | awk '{print $1}')
echo "SHA256: $SHA256"

# Create source.json
echo "Creating source.json..."
cat > "$MODULE_DIR/source.json" <<EOF
{
  "integrity": "sha256-$SHA256",
  "strip_prefix": "envoy-$VERSION",
  "url": "$ARCHIVE_URL"
}
EOF

# Update or create metadata.json
METADATA_FILE="$BCR_PATH/modules/envoy/metadata.json"
if [ -f "$METADATA_FILE" ]; then
    echo "Updating existing metadata.json..."
    # Add version to the versions array if not already present
    if ! grep -q "\"$VERSION\"" "$METADATA_FILE"; then
        # Use jq if available, otherwise manual edit
        if command -v jq >/dev/null 2>&1; then
            TMP_META=$(mktemp)
            jq ".versions += [\"$VERSION\"] | .versions |= sort" "$METADATA_FILE" > "$TMP_META"
            mv "$TMP_META" "$METADATA_FILE"
            echo "Added version $VERSION to metadata.json"
        else
            echo "Please manually add version $VERSION to $METADATA_FILE"
            echo "jq is not installed for automatic update"
        fi
    fi
else
    echo "Creating new metadata.json..."
    cp "$REPO_ROOT/.bcr/metadata.template.json" "$METADATA_FILE"
    # Update versions array
    if command -v jq >/dev/null 2>&1; then
        TMP_META=$(mktemp)
        jq ".versions = [\"$VERSION\"]" "$METADATA_FILE" > "$TMP_META"
        mv "$TMP_META" "$METADATA_FILE"
    else
        echo "Please manually add version $VERSION to $METADATA_FILE"
    fi
fi

echo ""
echo "BCR submission prepared successfully!"
echo ""
echo "Module directory: $MODULE_DIR"
echo "Files created:"
echo "  - MODULE.bazel"
echo "  - source.json"
echo "  - metadata.json (updated)"
echo ""
echo "Next steps:"
echo "1. Review the files in $MODULE_DIR"
echo "2. Remove any local_path_override or git_override from MODULE.bazel"
echo "3. Test locally: cd $BCR_PATH && bazelisk build //modules/envoy/$VERSION/..."
echo "4. Commit and push: cd $BCR_PATH && git add modules/envoy && git commit -m 'Add Envoy $VERSION'"
echo "5. Create a pull request to https://github.com/bazelbuild/bazel-central-registry"
echo ""
echo "For more information, see .bcr/README.md"

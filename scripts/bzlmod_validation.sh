#!/bin/bash

# Bazel WORKSPACE to bzlmod Migration Validation Script
# This script helps validate the hybrid setup works correctly

set -e

echo "=== Bazel WORKSPACE to bzlmod Migration Validation ==="
echo

# Check if bazel is available
if ! command -v bazel &> /dev/null; then
    echo "❌ Bazel not found. Please install Bazel first."
    exit 1
fi

echo "✅ Bazel found: $(bazel --version 2>/dev/null || echo 'version check failed')"
echo

# Test 1: Default WORKSPACE build
echo "🔍 Test 1: Default WORKSPACE build (bzlmod disabled)"
echo "Running: bazel query //..."

if bazel query //... >/dev/null 2>&1; then
    echo "✅ WORKSPACE build query successful"
else
    echo "❌ WORKSPACE build query failed"
    echo "This indicates issues with the WORKSPACE configuration"
fi
echo

# Test 2: Experimental bzlmod build  
echo "🔍 Test 2: Experimental bzlmod build"
echo "Running: bazel query --config=bzlmod //..."

if bazel query --config=bzlmod //... >/dev/null 2>&1; then
    echo "✅ bzlmod build query successful"
else
    echo "❌ bzlmod build query failed"
    echo "This indicates issues with the MODULE.bazel configuration"
fi
echo

# Test 3: Check for conflicting dependencies
echo "🔍 Test 3: Checking for dependency conflicts"

# Check if any patched dependencies are still in MODULE.bazel
conflicts_found=false
patched_deps=("abseil-cpp" "c-ares" "grpc" "googletest" "protobuf" "rules_rust" "rules_fuzzing" "rules_foreign_cc")

for dep in "${patched_deps[@]}"; do
    if grep -q "name.*$dep" MODULE.bazel && ! grep -q "# NOTE.*$dep.*requires patches" MODULE.bazel; then
        echo "❌ Conflict: $dep is in MODULE.bazel but requires patches"
        conflicts_found=true
    fi
done

if [ "$conflicts_found" = false ]; then
    echo "✅ No dependency conflicts found"
else
    echo "❌ Dependency conflicts detected - see above"
fi
echo

# Test 4: Validate patch files exist
echo "🔍 Test 4: Validating patch files exist"

missing_patches=false
# Get all patch files referenced in repositories.bzl
patch_refs=$(grep -o '@envoy//bazel[^"]*\.patch' bazel/repositories.bzl | sed 's/@envoy\/\///g' | sort -u)

for patch_file in $patch_refs; do
    if [ ! -f "$patch_file" ]; then
        echo "❌ Missing patch file: $patch_file"
        missing_patches=true
    fi
done

if [ "$missing_patches" = false ]; then
    echo "✅ All referenced patch files found"
else
    echo "❌ Missing patch files detected"
fi
echo

echo "=== Migration Validation Summary ==="
echo "📋 Next Steps:"
echo "1. Run 'bazel build <target>' to test WORKSPACE mode"
echo "2. Run 'bazel build --config=bzlmod <target>' to test hybrid mode"
echo "3. Compare build outputs to ensure equivalence"
echo "4. Gradually migrate non-patched dependencies to bzlmod"
echo
echo "📚 See BZLMOD_MIGRATION.md for detailed migration strategy"
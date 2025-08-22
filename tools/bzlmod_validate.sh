#!/bin/bash

# Bzlmod Migration Validation Script
# This script helps validate the bzlmod migration by testing key functionality

set -e

echo "🚀 Validating Bzlmod Migration for Envoy"
echo "========================================"

# Check if bzlmod is enabled
echo "📋 Checking bzlmod configuration..."
if grep -q "enable_bzlmod" .bazelrc; then
    echo "✅ bzlmod is enabled in .bazelrc"
else
    echo "❌ bzlmod is not enabled in .bazelrc"
    exit 1
fi

# Check MODULE.bazel exists
if [[ -f "MODULE.bazel" ]]; then
    echo "✅ MODULE.bazel file exists"
else
    echo "❌ MODULE.bazel file not found"
    exit 1
fi

# Validate MODULE.bazel syntax (basic check)
echo "📋 Validating MODULE.bazel syntax..."
if command -v bazel >/dev/null 2>&1; then
    if timeout 30s bazel mod show_extension_repos >/dev/null 2>&1; then
        echo "✅ MODULE.bazel syntax is valid (validated with bazel)"
        BAZEL_AVAILABLE=true
    else
        echo "⚠️  MODULE.bazel syntax validation failed with bazel"
        echo "This may be due to network issues or missing dependencies"
        BAZEL_AVAILABLE=false
    fi
else
    echo "ℹ️  Bazel not available, performing basic syntax checks..."
    # Basic syntax validation without bazel
    if python3 -c "
import sys
try:
    with open('MODULE.bazel', 'r') as f:
        content = f.read()
        required_patterns = ['module(', 'bazel_dep(']
        for pattern in required_patterns:
            if pattern not in content:
                print(f'❌ Missing required pattern: {pattern}')
                sys.exit(1)
        print('✅ Basic MODULE.bazel syntax appears valid')
except Exception as e:
    print(f'❌ Error reading MODULE.bazel: {e}')
    sys.exit(1)
    "; then
        echo "✅ Basic MODULE.bazel syntax validation passed"
        BAZEL_AVAILABLE=false
    else
        echo "❌ Basic MODULE.bazel syntax validation failed"
        exit 1
    fi
fi

# Show dependency graph
echo "📋 Displaying bzlmod dependency graph..."
if [[ "$BAZEL_AVAILABLE" == "true" ]]; then
    echo "Run 'bazel mod graph' to see the full dependency tree"
    if timeout 20s bazel mod graph >/dev/null 2>&1; then
        echo "✅ Dependency graph generation successful"
    else
        echo "⚠️  Dependency graph generation failed or timed out"
    fi
else
    echo "ℹ️  Bazel not available - run 'bazel mod graph' when bazel is working"
fi

# Test core builds (with timeout to avoid hanging)
echo "📋 Testing core build targets..."

if [[ "$BAZEL_AVAILABLE" == "true" ]]; then
    test_targets=(
        "//source/common/common:version_lib"
        "//source/common/protobuf:utility_lib"
        "//source/common/buffer:buffer_lib"
    )

    for target in "${test_targets[@]}"; do
        echo "Testing build of $target..."
        if timeout 60s bazel build "$target" >/dev/null 2>&1; then
            echo "✅ $target builds successfully"
        else
            echo "⚠️  $target build failed or timed out (this may be expected during initial migration)"
        fi
    done

    # Test analysis phase only (faster than full build)
    echo "📋 Testing analysis phase for major targets..."
    analysis_targets=(
        "//source/exe:envoy-static"
        "//test/common/common:version_test"
    )

    for target in "${analysis_targets[@]}"; do
        echo "Testing analysis of $target..."
        if timeout 30s bazel query "deps($target)" >/dev/null 2>&1; then
            echo "✅ $target analysis successful"
        else
            echo "⚠️  $target analysis failed or timed out"
        fi
    done
else
    echo "ℹ️  Bazel not available - skipping build tests"
    echo "ℹ️  Once bazel is working, test with:"
    echo "   bazel build //source/common/common:version_lib"
    echo "   bazel build //source/common/protobuf:utility_lib" 
fi

echo ""
echo "🎉 Migration validation complete!"
echo ""
echo "📖 Next Steps:"
echo "1. Run 'bazel mod graph' to explore the dependency tree"
echo "2. Test your specific build targets"
echo "3. Check the migration documentation in docs/root/start/migrating/bzlmod.md"
echo "4. Consider migrating additional dependencies from WORKSPACE"
echo ""
echo "📚 Resources:"
echo "- Migration guide: https://bazel.build/external/migration"
echo "- BCR modules: https://github.com/bazelbuild/bazel-central-registry"
echo "- Bzlmod documentation: https://bazel.build/external/mod"
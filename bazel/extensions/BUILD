"""Build file for Envoy bzlmod extensions."""

package(default_visibility = ["//visibility:public"])

# This directory contains bzlmod extensions for Envoy's repository setup.
# Extensions are organized into:
#   - public/: Extensions for general use (main module, submodules, external consumers)
#   - private/: Internal-only extensions (currently unused)
#
# Extension naming follows simplified conventions:
#   - dependencies: Core dependency setup
#   - dependency_imports: Toolchain imports and registrations  
#   - api_*: API-related extensions
#   - envoy_mobile*: Mobile-specific extensions
#   - envoy_android: Android-specific configuration
#
# Usage patterns:
#   Main module: use_extension("//bazel/extensions/public:NAME.bzl", "NAME")
#   Submodules: use_extension("@envoy//bazel/extensions/public:NAME.bzl", "NAME")
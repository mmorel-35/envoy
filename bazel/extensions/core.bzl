"""Consolidated extension for Envoy's core dependencies.

This extension consolidates the functionality of dependencies.bzl and dependencies_extra.bzl
to reduce extension proliferation and improve maintainability as recommended in 
BZLMOD_RECOMMENDATIONS.md.
"""

load("//bazel:repositories.bzl", "envoy_dependencies")
load("@proxy_wasm_cpp_host//bazel/cargo/wasmtime/remote:crates.bzl", "crate_repositories")
load("@com_google_protobuf//bazel/private:proto_bazel_features.bzl", "proto_bazel_features")
load("@envoy_examples//bazel:env.bzl", "envoy_examples_env")
load("//bazel/external/cargo:crates.bzl", "raze_fetch_remote_crates")

def _python_minor_version(python_version):
    return "_".join(python_version.split(".")[:-1])

PYTHON_VERSION = "3.12.3"
PYTHON_MINOR_VERSION = _python_minor_version(PYTHON_VERSION)

def _core_impl(module_ctx, python_version = PYTHON_VERSION, ignore_root_user_error = False):
    """Implementation for core extension.

    This extension consolidates Envoy's core dependency setup, combining:
    - Main dependencies (from dependencies.bzl)
    - Extra dependencies (from dependencies_extra.bzl)
    
    This reduces extension proliferation from 2 separate extensions to 1,
    following bzlmod best practices for extension consolidation.
    """

    # Core dependencies setup (from dependencies.bzl)
    envoy_dependencies()

    # Extra dependencies setup (from dependencies_extra.bzl)
    raze_fetch_remote_crates()
    crate_repositories()

    if not native.existing_rule("proto_bazel_features"):
        proto_bazel_features(name = "proto_bazel_features")

    envoy_examples_env()

# Consolidated module extension for Envoy core dependencies
core = module_extension(
    implementation = _core_impl,
    doc = """
    Consolidated extension for Envoy's core dependencies.
    
    This extension combines the functionality of:
    - dependencies.bzl: Main Envoy dependencies with patches and complex setup
    - dependencies_extra.bzl: Additional dependencies and crate repositories
    
    Provides repositories:
    - All repositories from envoy_dependencies() (100+ repos)
    - Rust crate repositories via raze_fetch_remote_crates()
    - proto_bazel_features for protobuf integration
    - envoy_examples environment setup
    
    Benefits of consolidation:
    - Reduced extension count (2 → 1 for core deps)
    - Simplified dependency graph 
    - Easier maintenance and debugging
    - Better alignment with bzlmod best practices
    """,
)
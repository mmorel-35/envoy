"""Consolidated extension for Envoy's toolchains and imports.

This extension consolidates the functionality of dependency_imports.bzl, 
dependency_imports_extra.bzl, and repo.bzl to reduce extension proliferation 
and improve maintainability as recommended in BZLMOD_RECOMMENDATIONS.md.
"""

load("//bazel:dependency_imports.bzl", "envoy_dependency_imports")
load("//bazel:dependency_imports_extra.bzl", "envoy_dependency_imports_extra")
load("//bazel:repo.bzl", "envoy_repo")

def _toolchains_impl(module_ctx):
    """Implementation for toolchains extension.

    This extension consolidates Envoy's toolchain and import setup, combining:
    - Dependency imports (from dependency_imports.bzl)
    - Extra dependency imports (from dependency_imports_extra.bzl)  
    - Repository setup (from repo.bzl)
    
    This reduces extension proliferation from 3 separate extensions to 1,
    following bzlmod best practices for extension consolidation.
    """

    # Main dependency imports setup
    envoy_dependency_imports()
    
    # Extra dependency imports setup  
    envoy_dependency_imports_extra()
    
    # Repository metadata setup
    envoy_repo()

# Consolidated module extension for Envoy toolchains and imports
toolchains = module_extension(
    implementation = _toolchains_impl,
    doc = """
    Consolidated extension for Envoy's toolchains and imports.
    
    This extension combines the functionality of:
    - dependency_imports.bzl: Main toolchain imports and registrations
    - dependency_imports_extra.bzl: Additional dependency imports
    - repo.bzl: Repository metadata and tooling setup
    
    Handles:
    - Go toolchain registration and dependencies
    - Python pip dependencies (base, dev, fuzzing)
    - Rust toolchain and crate universe setup
    - Foreign CC dependencies and toolchains
    - Apple, shellcheck, and other development toolchains
    - Repository metadata and tooling configuration
    
    Benefits of consolidation:
    - Reduced extension count (3 → 1 for toolchains)
    - Simplified dependency graph
    - Easier maintenance and debugging  
    - Better alignment with bzlmod best practices
    - Single point of configuration for all toolchains
    """,
)
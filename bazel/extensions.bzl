"""Non-BCR dependencies for Envoy.

This extension provides repositories that are not available in Bazel Central Registry (BCR).
"""

load(":repositories.bzl", "envoy_dependencies")
load(":repositories_extra.bzl", "envoy_dependencies_extra")
load("@envoy_toolshed//compile:sanitizer_libs.bzl", "setup_sanitizer_libs")
load("@envoy_toolshed//coverage/grcov:grcov_repository.bzl", "grcov_repository")
load("@rules_fuzzing//fuzzing:repositories.bzl", "rules_fuzzing_dependencies")
load("@com_google_cel_cpp//bazel:deps.bzl", "parser_deps")
load(":repo.bzl", "envoy_repo")

def _non_module_deps_impl(module_ctx):
    """Implementation for non_module_deps extension.

    This extension calls envoy_dependencies(bzlmod=True) and related setup functions
    which create repositories not in BCR. It safely coexists with BCR deps because
    envoy_http_archive checks native.existing_rules() before creating repositories.

    Args:
        module_ctx: Module extension context
    """
    # Core dependencies - skips repositories that already exist from BCR
    envoy_dependencies()
    envoy_dependencies_extra()
    
    # Toolchain and import setup
    grcov_repository()
    setup_sanitizer_libs()
    
    # Fuzzing dependencies
    if not native.existing_rule("rules_fuzzing_oss_fuzz"):
        rules_fuzzing_dependencies(
            oss_fuzz = True,
            honggfuzz = False,
        )
    
    # Parser dependencies (CEL)
    parser_deps()
    
    # Repository metadata setup
    envoy_repo()

non_module_deps = module_extension(
    implementation = _non_module_deps_impl,
    doc = """
    Extension for Envoy dependencies not available in BCR.

    This extension creates repositories for dependencies not yet available in BCR
    or requiring custom build files. It calls envoy_dependencies() and related
    setup functions which are smart enough to skip repositories that already exist.

    For WORKSPACE mode, call envoy_dependencies() and related functions directly.
    This extension should only be used in MODULE.bazel files.
    """,
)

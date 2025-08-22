"""Extension for Envoy's extra dependencies."""

load("@proxy_wasm_cpp_host//bazel/cargo/wasmtime/remote:crates.bzl", "crate_repositories")
load("@com_google_protobuf//bazel/private:proto_bazel_features.bzl", "proto_bazel_features")
load("@envoy_examples//bazel:env.bzl", "envoy_examples_env")
load("//bazel/external/cargo:crates.bzl", "raze_fetch_remote_crates")

def _python_minor_version(python_version):
    return "_".join(python_version.split(".")[:-1])

PYTHON_VERSION = "3.12.3"
PYTHON_MINOR_VERSION = _python_minor_version(PYTHON_VERSION)


def _dependencies_extra_impl(module_ctx, python_version = PYTHON_VERSION,
        ignore_root_user_error = False):
    """Implementation for dependencies_extra extension.

    This extension wraps the envoy_dependencies_extra() function to make it
    available as a bzlmod module extension.
    """

    # Call the extra dependencies function
    raze_fetch_remote_crates()
    crate_repositories()

    if not native.existing_rule("proto_bazel_features"):
        proto_bazel_features(name = "proto_bazel_features")

    envoy_examples_env()

# Module extension for envoy_dependencies_extra
dependencies_extra = module_extension(
    implementation = _dependencies_extra_impl,
    doc = """
    Extension for Envoy's extra dependencies.
    
    This extension wraps the envoy_dependencies_extra() function to make it
    available as a bzlmod module extension. These are dependencies that rely  
    on a first stage of dependency loading in envoy_dependencies().
    """,
)

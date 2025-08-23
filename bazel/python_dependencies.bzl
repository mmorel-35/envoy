load("@com_google_protobuf//bazel:system_python.bzl", "system_python")
load("@envoy_toolshed//:packages.bzl", "load_packages")
load("@rules_python//python:pip.bzl", "pip_parse")

def envoy_python_dependencies():
    # TODO(phlax): rename base_pip3 -> pip3 and remove this
    load_packages()
    pip_parse(
        name = "base_pip3",
        python_interpreter_target = "@python3_12_host//:python",
        requirements_lock = "@envoy//tools/base:requirements.txt",
        extra_pip_args = ["--require-hashes"],
    )

    pip_parse(
        name = "dev_pip3",
        python_interpreter_target = "@python3_12_host//:python",
        requirements_lock = "@envoy//tools/dev:requirements.txt",
        extra_pip_args = ["--require-hashes"],
    )

    pip_parse(
        name = "fuzzing_pip3",
        python_interpreter_target = "@python3_12_host//:python",
        requirements_lock = "@rules_fuzzing//fuzzing:requirements.txt",
        extra_pip_args = ["--require-hashes"],
    )

    system_python(
        name = "system_python",
        minimum_python_version = "3.7",
    )

def _envoy_python_dependencies_impl(module_ctx):
    """Implementation of the envoy_python_dependencies extension."""
    # Call the python dependencies function
    envoy_python_dependencies()

# Module extension for envoy_python_dependencies following consistent naming convention
envoy_python_dependencies_ext = module_extension(
    implementation = _envoy_python_dependencies_impl,
    doc = """
    Extension for Envoy's Python dependencies.
    
    This extension wraps the envoy_python_dependencies() function to make it
    available as a bzlmod module extension, following the consistent
    naming convention envoy_*_ext across all Envoy modules.
    """,
)

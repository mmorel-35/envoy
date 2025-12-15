load("@com_google_protobuf//python/dist:system_python.bzl", "system_python")
load("@envoy_toolshed//:packages.bzl", "load_packages")
load("@rules_python//python:pip.bzl", "pip_parse")

# Default hermetic Python interpreter for Linux x86_64
_DEFAULT_PYTHON_INTERPRETER_TARGET = "@python3_12_x86_64-unknown-linux-gnu//:bin/python3"

def envoy_python_dependencies(python_interpreter_target = _DEFAULT_PYTHON_INTERPRETER_TARGET):
    """Configure Python dependencies for Envoy.
    
    Args:
        python_interpreter_target: The hermetic Python interpreter to use for pip_parse.
            Defaults to Linux x86_64. For macOS x86_64, use:
            "@python3_12_x86_64-apple-darwin//:bin/python3"
            For macOS ARM64, use:
            "@python3_12_aarch64-apple-darwin//:bin/python3"
            For Windows x86_64, use:
            "@python3_12_x86_64-pc-windows-msvc//:python.exe"
    """
    # TODO(phlax): rename base_pip3 -> pip3 and remove this
    load_packages()
    
    # Use hermetic Python toolchain for pip_parse.
    # Note: pip_parse runs before toolchain resolution, so it cannot automatically
    # use the registered hermetic toolchain. We must explicitly specify the
    # python_interpreter_target pointing to the platform-specific interpreter.
    # TODO: Use multi_pip_parse for proper cross-platform support with multiple interpreters.
    pip_parse(
        name = "base_pip3",
        requirements_lock = "@envoy//tools/base:requirements.txt",
        extra_pip_args = ["--require-hashes"],
        python_interpreter_target = python_interpreter_target,
    )

    pip_parse(
        name = "dev_pip3",
        requirements_lock = "@envoy//tools/dev:requirements.txt",
        extra_pip_args = ["--require-hashes"],
        python_interpreter_target = python_interpreter_target,
    )

    system_python(
        name = "system_python",
        minimum_python_version = "3.7",
    )

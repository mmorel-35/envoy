load("@com_google_protobuf//python/dist:system_python.bzl", "system_python")
load("@envoy_toolshed//:packages.bzl", "load_packages")
load("@rules_python//python:pip.bzl", "pip_parse")

def envoy_python_dependencies():
    # TODO(phlax): rename base_pip3 -> pip3 and remove this
    load_packages()
    
    # Use hermetic Python toolchain for pip_parse.
    # Note: pip_parse runs before toolchain resolution, so it cannot automatically
    # use the registered hermetic toolchain. We must explicitly specify the
    # python_interpreter_target pointing to the platform-specific interpreter.
    # Currently configured for Linux x86_64 (the primary build platform).
    # For macOS builds, this should be changed to:
    #   "@python3_12_x86_64-apple-darwin//:bin/python3" or "@python3_12_aarch64-apple-darwin//:bin/python3"
    # TODO: Use multi_pip_parse for proper cross-platform support with multiple interpreters.
    pip_parse(
        name = "base_pip3",
        requirements_lock = "@envoy//tools/base:requirements.txt",
        extra_pip_args = ["--require-hashes"],
        python_interpreter_target = "@python3_12_x86_64-unknown-linux-gnu//:bin/python3",
    )

    pip_parse(
        name = "dev_pip3",
        requirements_lock = "@envoy//tools/dev:requirements.txt",
        extra_pip_args = ["--require-hashes"],
        python_interpreter_target = "@python3_12_x86_64-unknown-linux-gnu//:bin/python3",
    )

    system_python(
        name = "system_python",
        minimum_python_version = "3.7",
    )

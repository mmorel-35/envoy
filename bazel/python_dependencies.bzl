load("@com_google_protobuf//python/dist:system_python.bzl", "system_python")
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

    # fuzzing_pip3 uses @rules_fuzzing's requirements.txt but with Envoy's Python interpreter.
    # This is necessary because rules_fuzzing_init() doesn't support specifying a custom
    # Python interpreter. The repo_mapping in repositories.bzl redirects @fuzzing_py_deps
    # (which rules_fuzzing would create) to this @fuzzing_pip3 repository.
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

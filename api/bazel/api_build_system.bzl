# This file is deprecated and maintained for backward compatibility only.
# Please use the individual rule files directly instead.
load(
    ":api_cc_py_proto_library.bzl",
    _EnvoyProtoDepsInfo = "EnvoyProtoDepsInfo",
    _api_cc_py_proto_library = "api_cc_py_proto_library",
)
load(
    ":api_cc_test.bzl",
    _api_cc_test = "api_cc_test",
)
load(
    ":api_go_test.bzl",
    _api_go_test = "api_go_test",
)
load(
    ":api_proto_package.bzl",
    _api_proto_package = "api_proto_package",
)

# Re-export for backward compatibility
EnvoyProtoDepsInfo = _EnvoyProtoDepsInfo
api_cc_py_proto_library = _api_cc_py_proto_library
api_cc_test = _api_cc_test
api_go_test = _api_go_test
api_proto_package = _api_proto_package

# This file is deprecated and maintained for backward compatibility only.
# Please use rules.bzl directly instead.
load(
    ":rules.bzl",
    _EnvoyProtoDepsInfo = "EnvoyProtoDepsInfo",
    _api_cc_py_proto_library = "api_cc_py_proto_library",
    _api_cc_test = "api_cc_test",
    _api_go_test = "api_go_test",
    _api_proto_package = "api_proto_package",
)

# Re-export for backward compatibility
EnvoyProtoDepsInfo = _EnvoyProtoDepsInfo
api_cc_py_proto_library = _api_cc_py_proto_library
api_cc_test = _api_cc_test
api_go_test = _api_go_test
api_proto_package = _api_proto_package

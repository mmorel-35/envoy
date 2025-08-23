def _default_envoy_api_impl(ctx):
    ctx.file("WORKSPACE", "")
    api_dirs = [
        "BUILD",
        "bazel",
        "envoy",
        "examples",
        "test",
        "tools",
        "versioning",
        "contrib",
        "buf.yaml",
    ]
    for d in api_dirs:
        ctx.symlink(ctx.path(ctx.attr.envoy_root).dirname.get_child(ctx.attr.reldir).get_child(d), d)

_default_envoy_api = repository_rule(
    implementation = _default_envoy_api_impl,
    attrs = {
        "envoy_root": attr.label(default = "@envoy//:BUILD"),
        "reldir": attr.string(),
    },
)

def envoy_api_binding():
    # Treat the data plane API as an external repo, this simplifies exporting
    # the API to https://github.com/envoyproxy/data-plane-api.
    if "envoy_api" not in native.existing_rules().keys():
        _default_envoy_api(name = "envoy_api", reldir = "api")

    # Detect bzlmod context to skip native.bind() calls
    _IS_BZLMOD = str(Label("//:invalid")).startswith("@@")
    if not _IS_BZLMOD:
        # TODO(https://github.com/envoyproxy/envoy/issues/7719) need to remove both bindings and use canonical rules
        native.bind(
            name = "api_httpbody_protos",
            actual = "@com_google_googleapis//google/api:httpbody_cc_proto",
        )
        native.bind(
            name = "http_api_protos",
            actual = "@com_google_googleapis//google/api:annotations_cc_proto",
        )

def _envoy_api_binding_impl(module_ctx):
    """Implementation of the envoy_api_binding extension."""
    # Call the api binding function
    envoy_api_binding()

# Module extension for envoy_api_binding following consistent naming convention
envoy_api_binding_ext = module_extension(
    implementation = _envoy_api_binding_impl,
    doc = """
    Extension for Envoy's API binding setup.
    
    This extension wraps the envoy_api_binding() function to make it
    available as a bzlmod module extension, following the consistent
    naming convention envoy_*_ext across all Envoy modules.
    """,
)

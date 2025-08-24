load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def envoy_http_archive(name, locations, location_name = None, **kwargs):
    # `existing_rule_keys` contains the names of repositories that have already
    # been defined in the Bazel workspace. By skipping repos with existing keys,
    # users can override dependency versions by using standard Bazel repository
    # rules in their WORKSPACE files.
    if name not in native.existing_rules():
        location = locations[location_name or name]

        # Filter out repo_mapping for WORKSPACE compatibility.
        # The repo_mapping attribute is only supported in bzlmod module extensions,
        # not in native Bazel rules used in WORKSPACE builds. Since this function
        # is primarily used in WORKSPACE context (bzlmod extensions typically call
        # http_archive directly), we filter it out to avoid build errors.
        #
        # This ensures that code using external_http_archive with repo_mapping
        # (like _rules_fuzzing(), _com_google_absl(), etc.) works correctly in
        # both WORKSPACE and bzlmod contexts.
        filtered_kwargs = {}
        for key, value in kwargs.items():
            if key != "repo_mapping":
                filtered_kwargs[key] = value

        # HTTP tarball at a given URL. Add a BUILD file if requested.
        http_archive(
            name = name,
            urls = location["urls"],
            sha256 = location["sha256"],
            strip_prefix = location.get("strip_prefix", ""),
            **filtered_kwargs
        )

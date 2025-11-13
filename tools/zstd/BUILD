licenses(["notice"])  # Apache 2

# Alias to the native zstd_cli from @com_github_facebook_zstd.
# This maintains backward compatibility while using the BCR-compatible BUILD file.
alias(
    name = "zstd",
    actual = "@com_github_facebook_zstd//:zstd_cli",
    visibility = ["//visibility:public"],
)

load(":zstd.bzl", "zstd")

licenses(["notice"])  # Apache 2

# Now using the native zstd_cli from @com_github_facebook_zstd instead of
# the foreign_cc cmake build. This provides the same zstd binary functionality
# using the BCR-compatible BUILD file.
alias(
    name = "zstd",
    actual = "@com_github_facebook_zstd//:zstd_cli",
    visibility = ["//visibility:public"],
)

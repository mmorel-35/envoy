licenses(["notice"])  # Apache 2

genrule(
    name = "zstd_bin",
    srcs = ["@com_github_facebook_zstd//:zstd_cli"],
    outs = ["zstd"],
    cmd = "cp $< $@ && chmod +x $@",
    executable = True,
    visibility = ["//visibility:public"],
)

load("@rules_cc//cc:defs.bzl", "cc_library")

licenses(["notice"])  # BSD-3-Clause

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "zstd",
    srcs = glob([
        "lib/common/*.c",
        "lib/compress/*.c",
        "lib/decompress/*.c",
        "lib/dictBuilder/*.c",
    ]) + select({
        "@platforms//cpu:x86_64": glob(["lib/decompress/*.S"]),
        "//conditions:default": [],
    }),
    hdrs = glob([
        "lib/*.h",
        "lib/common/*.h",
        "lib/compress/*.h",
        "lib/decompress/*.h",
        "lib/dictBuilder/*.h",
    ]),
    defines = select({
        "@platforms//cpu:x86_64": [],
        "//conditions:default": ["ZSTD_DISABLE_ASM"],
    }),
    includes = [
        "lib",
        "lib/common",
    ],
    local_defines = [
        "XXH_NAMESPACE=ZSTD_",
        "ZSTD_MULTITHREAD",
    ],
    linkopts = select({
        "@platforms//os:windows": [],
        "//conditions:default": ["-pthread"],
    }),
)

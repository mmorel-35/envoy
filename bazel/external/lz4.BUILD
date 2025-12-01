load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_cc//cc:cc_static_library.bzl", "cc_static_library")

cc_library(
    name = "lz4",
    srcs = [
        "lib/lz4.c",
        "lib/xxhash.c",
        "lib/xxhash.h",
    ],
    hdrs = [
        "lib/lz4.h",
    ],
    strip_include_prefix = "lib/",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "lz4_lz4c_include",
    hdrs = [
        "lib/lz4.c",
    ],
    strip_include_prefix = "lib/",
)

cc_library(
    name = "lz4_hc",
    srcs = [
        "lib/lz4hc.c",
    ],
    hdrs = [
        "lib/lz4hc.h",
    ],
    strip_include_prefix = "lib/",
    visibility = ["//visibility:public"],
    deps = [
        ":lz4",
        ":lz4_lz4c_include",
    ],
)

cc_library(
    name = "lz4_file",
    srcs = [
        "lib/lz4file.c",
    ],
    hdrs = [
        "lib/lz4file.h",
    ],
    strip_include_prefix = "lib/",
    visibility = ["//:__subpackages__"],
    deps = [
        ":lz4",
        ":lz4_frame",
    ],
)

cc_library(
    name = "lz4_frame",
    srcs = [
        "lib/lz4frame.c",
        "lib/lz4frame_static.h",
    ],
    hdrs = [
        "lib/lz4frame.h",
    ],
    strip_include_prefix = "lib/",
    visibility = ["//visibility:public"],
    deps = [
        ":lz4",
        ":lz4_hc",
    ],
)

cc_library(
    name = "lz4_xxhash_include",
    hdrs = [
        "lib/xxhash.h",
    ],
    strip_include_prefix = "lib/",
    visibility = ["//:__subpackages__"],
)

# Combined library with all lz4 components needed for foreign_cc dependencies.
# This includes lz4, lz4_hc, and lz4_frame which provides LZ4F_compressFrame.
cc_library(
    name = "lz4_all",
    visibility = ["//visibility:public"],
    deps = [
        ":lz4",
        ":lz4_frame",
        ":lz4_hc",
    ],
)

# Create a proper static library archive from the cc_library.
cc_static_library(
    name = "lz4_static",
    deps = [":lz4_all"],
)

# Create a properly named liblz4.a archive for foreign_cc dependencies.
# The cc_library above produces Bazel-internal archives, but foreign_cc
# configure_make rules (like qatzip) expect a traditional liblz4.a file
# that can be found with -llz4.
genrule(
    name = "lz4_archive",
    srcs = [":lz4_static"],
    outs = ["lib/liblz4.a"],
    cmd = "mkdir -p $$(dirname $@) && cp $< $@",
    visibility = ["//visibility:public"],
)

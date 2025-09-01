licenses(["notice"])  # BSD-3-Clause

package(default_visibility = ["//visibility:public"])

# Export autotools files needed for configure_make with autogen = True
exports_files([
    "autogen.sh",
    "configure.ac",
    "configure.in",
    "Makefile.am",
    "Makefile.in",
])

# Export all files to make them available for foreign_cc
filegroup(
    name = "all",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)

# Main tcmalloc target that matches the upstream BUILD.bazel
# This allows the configure_make rule to reference specific targets while
# having access to all the autotools files
cc_library(
    name = "tcmalloc", 
    visibility = ["//visibility:public"],
    # Empty implementation - this is just a placeholder since configure_make
    # will build the actual library from source using autotools
)
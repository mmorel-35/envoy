"""Module extensions for Envoy's non-module dependencies.

This file defines module extensions to support Envoy's bzlmod migration while
respecting existing WORKSPACE patches and custom BUILD files. Each dependency
loaded here is not yet available as a Bazel module in the Bazel Central Registry
or requires custom configuration/patches specific to Envoy.
"""

load("@envoy_api//bazel:envoy_http_archive.bzl", "envoy_http_archive")
load("@envoy_api//bazel:external_deps.bzl", "load_repository_locations")
load(":repository_locations.bzl", "PROTOC_VERSIONS", "REPOSITORY_LOCATIONS_SPEC")

# Make all contents of an external repository accessible under a filegroup.
def _build_all_content(exclude = []):
    return """filegroup(name = "all", srcs = glob(["**"], exclude={}), visibility = ["//visibility:public"])""".format(repr(exclude))

BUILD_ALL_CONTENT = _build_all_content()
REPOSITORY_LOCATIONS = load_repository_locations(REPOSITORY_LOCATIONS_SPEC)

# Use this macro to reference any HTTP archive from bazel/repository_locations.bzl.
def _external_http_archive(name, **kwargs):
    envoy_http_archive(
        name,
        locations = REPOSITORY_LOCATIONS,
        **kwargs
    )

def _envoy_dependencies_impl(module_ctx):
    """Implementation of the envoy_dependencies module extension.
    
    This extension loads all non-module Envoy dependencies, applying the same
    patches and configurations used in WORKSPACE mode to ensure compatibility.
    
    Args:
        module_ctx: The module extension context
    """
    
    # BoringSSL FIPS variant with patches
    _external_http_archive(
        name = "boringssl_fips",
        location_name = "boringssl",
        build_file = "@envoy//bazel/external:boringssl_fips.BUILD",
        patches = ["@envoy//bazel:boringssl_fips.patch"],
        patch_args = ["-p1"],
    )
    
    # BoringSSL FIPS build dependencies
    NINJA_BUILD_CONTENT = "%s\nexports_files([\"configure.py\"])" % BUILD_ALL_CONTENT
    _external_http_archive(
        name = "fips_ninja",
        build_file_content = NINJA_BUILD_CONTENT,
    )
    
    CMAKE_BUILD_CONTENT = "%s\nexports_files([\"bin/cmake\"])" % BUILD_ALL_CONTENT
    _external_http_archive(
        name = "fips_cmake_linux_x86_64",
        build_file_content = CMAKE_BUILD_CONTENT,
    )
    _external_http_archive(
        name = "fips_cmake_linux_aarch64",
        build_file_content = CMAKE_BUILD_CONTENT,
    )
    
    GO_BUILD_CONTENT = "%s\nexports_files([\"bin/go\"])" % _build_all_content(["test/**"])
    _external_http_archive(
        name = "fips_go_linux_amd64",
        build_file_content = GO_BUILD_CONTENT,
    )
    _external_http_archive(
        name = "fips_go_linux_arm64",
        build_file_content = GO_BUILD_CONTENT,
    )
    
    # AWS-LC as an alternative SSL library
    _external_http_archive(
        name = "aws_lc",
        build_file = "@envoy//bazel/external:aws_lc.BUILD",
    )
    
    # C++ dependencies with custom BUILD files
    _external_http_archive("grpc_httpjson_transcoding")
    
    _external_http_archive(
        name = "com_google_protoconverter",
        patch_args = ["-p1"],
        patches = ["@envoy//bazel:com_google_protoconverter.patch"],
        patch_cmds = [
            "rm src/google/protobuf/stubs/common.cc",
            "rm src/google/protobuf/stubs/common.h",
            "rm src/google/protobuf/stubs/common_unittest.cc",
            "rm src/google/protobuf/util/converter/port_def.inc",
            "rm src/google/protobuf/util/converter/port_undef.inc",
        ],
    )
    
    _external_http_archive("com_google_protofieldextraction")
    
    _external_http_archive(
        "com_google_protoprocessinglib",
        patch_args = ["-p1"],
        patches = ["@envoy//bazel:proto_processing_lib.patch"],
    )
    
    _external_http_archive("ocp")
    
    # Dependencies with custom BUILD files
    _external_http_archive(
        name = "com_github_openhistogram_libcircllhist",
        build_file = "@envoy//bazel/external:libcircllhist.BUILD",
    )
    
    _external_http_archive(
        name = "com_github_awslabs_aws_c_auth",
        build_file = "@envoy//bazel/external:aws-c-auth.BUILD",
    )
    
    _external_http_archive(
        name = "com_github_axboe_liburing",
        build_file = "@envoy//bazel/external:liburing.BUILD",
    )
    
    _external_http_archive(
        name = "com_github_c_ares_c_ares",
        build_file_content = BUILD_ALL_CONTENT,
        patches = ["@envoy//bazel:c-ares.patch"],
        patch_args = ["-p1"],
    )
    
    _external_http_archive(
        name = "com_github_envoyproxy_sqlparser",
        build_file_content = BUILD_ALL_CONTENT,
    )
    
    _external_http_archive(
        name = "com_github_mirror_tclap",
        build_file = "@envoy//bazel/external:tclap.BUILD",
    )
    
    _external_http_archive(
        name = "com_github_google_libprotobuf_mutator",
        build_file = "@envoy//bazel/external:libprotobuf_mutator.BUILD",
    )
    
    _external_http_archive(
        name = "com_github_google_libsxg",
        build_file_content = BUILD_ALL_CONTENT,
    )
    
    _external_http_archive(
        name = "com_github_unicode_org_icu",
        build_file = "@envoy//bazel/external:icu.BUILD",
        patches = ["@envoy//bazel/foreign_cc:icu.patch"],
        patch_args = ["-p1"],
        patch_cmds = ["rm source/common/unicode/uversion.h"],
    )
    
    _external_http_archive(
        name = "com_github_intel_ipp_crypto_crypto_mb",
        build_file_content = BUILD_ALL_CONTENT,
        patches = ["@envoy//bazel/foreign_cc:ipp-crypto.patch"],
        patch_args = ["-p1"],
    )
    
    _external_http_archive(
        name = "com_github_intel_qatlib",
        build_file_content = BUILD_ALL_CONTENT,
    )
    
    _external_http_archive(
        name = "com_github_intel_qatzip",
        build_file_content = BUILD_ALL_CONTENT,
    )
    
    _external_http_archive(
        name = "com_github_qat_zstd",
        build_file_content = BUILD_ALL_CONTENT,
        patches = ["@envoy//bazel/foreign_cc:qat_zstd.patch"],
        patch_args = ["-p1"],
    )
    
    _external_http_archive(
        name = "com_github_lz4_lz4",
        build_file_content = BUILD_ALL_CONTENT,
    )
    
    _external_http_archive(
        name = "com_github_libevent_libevent",
        build_file_content = BUILD_ALL_CONTENT,
    )
    
    _external_http_archive(
        name = "net_colm_open_source_colm",
        build_file_content = BUILD_ALL_CONTENT,
    )
    
    _external_http_archive(
        name = "net_colm_open_source_ragel",
        build_file_content = BUILD_ALL_CONTENT,
    )
    
    _external_http_archive(
        name = "com_github_zlib_ng_zlib_ng",
        build_file_content = BUILD_ALL_CONTENT,
    )
    
    _external_http_archive(
        name = "org_boost",
        build_file = "@envoy//bazel/external:boost.BUILD",
        patch_cmds = ["rm -rf doc"],
    )
    
    # V8 with multiple patches
    # Note: repo_mapping is handled differently in bzlmod mode
    _external_http_archive(
        name = "v8",
        patches = [
            "@envoy//bazel:v8.patch",
            "@envoy//bazel:v8_ppc64le.patch",
            "@envoy//bazel:v8_python.patch",
        ],
        patch_args = ["-p1"],
        patch_cmds = [
            "find ./src ./include -type f -exec sed -i.bak -e 's!#include \"third_party/simdutf/simdutf.h\"!#include \"simdutf.h\"!' {} \\;",
            "find ./src ./include -type f -exec sed -i.bak -e 's!#include \"third_party/fp16/src/include/fp16.h\"!#include \"fp16.h\"!' {} \\;",
            "find ./src ./include -type f -exec sed -i.bak -e 's!#include \"third_party/dragonbox/src/include/dragonbox/dragonbox.h\"!#include \"dragonbox/dragonbox.h\"!' {} \\;",
            "find ./src ./include -type f -exec sed -i.bak -e 's!#include \"third_party/fast_float/src/include/fast_float/!#include \"fast_float/!' {} \\;",
        ],
    )
    
    _external_http_archive(
        name = "dragonbox",
        build_file = "@envoy//bazel/external:dragonbox.BUILD",
    )
    
    _external_http_archive(
        name = "fp16",
        build_file = "@envoy//bazel/external:fp16.BUILD",
    )
    
    _external_http_archive(
        name = "simdutf",
        build_file = "@envoy//bazel/external:simdutf.BUILD",
    )
    
    _external_http_archive(
        name = "intel_ittapi",
        build_file = "@envoy//bazel/external:intel_ittapi.BUILD",
    )
    
    # QUICHE with custom BUILD
    _external_http_archive(
        name = "com_github_google_quiche",
        patch_cmds = ["find quiche/ -type f -name \"*.bazel\" -delete"],
        build_file = "@envoy//bazel/external:quiche.BUILD",
    )
    
    _external_http_archive(
        name = "googleurl",
        patches = ["@envoy//bazel/external:googleurl.patch"],
        patch_args = ["-p1"],
    )
    
    _external_http_archive(
        name = "org_llvm_releases_compiler_rt",
        build_file = "@envoy//bazel/external:compiler_rt.BUILD",
    )
    
    # gRPC with patches and dependencies
    # Note: repo_mapping is handled differently in bzlmod mode
    _external_http_archive(
        name = "com_github_grpc_grpc",
        patch_args = ["-p1"],
        patches = ["@envoy//bazel:grpc.patch"],
    )
    
    _external_http_archive(
        "build_bazel_rules_apple",
        patch_args = ["-p1"],
        patches = [
            "@envoy//bazel:rules_apple.patch",
            "@envoy//bazel:rules_apple_py.patch",
        ],
    )
    
    _external_http_archive("rules_proto_grpc")
    
    # Proxy WASM dependencies with patches
    _external_http_archive(
        name = "proxy_wasm_cpp_sdk",
        patch_args = ["-p1"],
        patches = [
            "@envoy//bazel:proxy_wasm_cpp_sdk.patch",
        ],
    )
    
    _external_http_archive(
        name = "proxy_wasm_cpp_host",
        patch_args = ["-p1"],
        patches = [
            "@envoy//bazel:proxy_wasm_cpp_host.patch",
        ],
    )
    
    _external_http_archive("proxy_wasm_rust_sdk")
    
    # JWT verify library with patches
    _external_http_archive(
        "com_github_google_jwt_verify",
        patches = ["@envoy//bazel:jwt_verify_lib.patch"],
        patch_args = ["-p1"],
    )
    
    # LuaJIT with patches
    _external_http_archive(
        name = "com_github_luajit_luajit",
        build_file_content = BUILD_ALL_CONTENT,
        patches = ["@envoy//bazel/foreign_cc:luajit.patch"],
        patch_args = ["-p1"],
    )
    
    # TCMalloc with patches
    _external_http_archive(
        name = "com_github_google_tcmalloc",
        patches = ["@envoy//bazel:tcmalloc.patch"],
        patch_args = ["-p1"],
    )
    
    # CEL C++ library
    _external_http_archive(
        name = "com_google_cel_cpp",
        build_file = "@envoy//bazel/external:cel-cpp.BUILD",
    )
    
    # Google Perfetto
    _external_http_archive(
        name = "com_github_google_perfetto",
        build_file = "@envoy//bazel/external:perfetto.BUILD",
    )
    
    # Network protocol libraries
    _external_http_archive(
        name = "com_github_nghttp2_nghttp2",
        build_file_content = BUILD_ALL_CONTENT,
        patches = ["@envoy//bazel/foreign_cc:nghttp2.patch"],
        patch_args = ["-p1"],
    )
    
    _external_http_archive(
        name = "com_github_msgpack_cpp",
        build_file = "@envoy//bazel/external:msgpack.BUILD",
    )
    
    # Hyperscan and Vectorscan (regex engines)
    _external_http_archive(
        name = "io_hyperscan",
        build_file_content = BUILD_ALL_CONTENT,
        patches = ["@envoy//bazel/foreign_cc:hyperscan.patch"],
        patch_args = ["-p1"],
        patch_cmds = ["rm -rf BUILD"],
    )
    
    _external_http_archive(
        name = "io_vectorscan",
        build_file_content = BUILD_ALL_CONTENT,
        patch_cmds = ["rm -rf BUILD"],
    )
    
    # OpenTelemetry
    _external_http_archive(
        name = "io_opentelemetry_cpp",
    )
    
    # Datadog tracing
    _external_http_archive(
        name = "com_github_datadog_dd_trace_cpp",
        build_file = "@envoy//bazel/external:dd_trace_cpp.BUILD",
    )
    
    # SkyWalking with patches
    _external_http_archive(
        name = "com_github_skyapm_cpp2sky",
        patches = ["@envoy//bazel:com_github_skyapm_cpp2sky.patch"],
        patch_args = ["-p1"],
        build_file = "@envoy//bazel/external:cpp2sky.BUILD",
    )
    
    _external_http_archive(
        name = "com_github_alibaba_hessian2_codec",
        build_file = "@envoy//bazel/external:hessian2_codec.BUILD",
    )
    
    _external_http_archive(
        name = "com_github_ncopa_suexec",
        build_file = "@envoy//bazel/external:suexec.BUILD",
    )
    
    # MaxMind DB library
    _external_http_archive(
        name = "com_github_maxmind_libmaxminddb",
        build_file_content = BUILD_ALL_CONTENT,
    )
    
    # Protoc binaries for different platforms
    for platform in PROTOC_VERSIONS:
        _external_http_archive(
            "com_google_protobuf_protoc_%s" % platform,
            build_file = "@envoy//bazel/protoc:BUILD.protoc",
        )
    
    # Additional tooling and build dependencies
    _external_http_archive("bazel_toolchains")
    _external_http_archive("bazel_compdb")
    _external_http_archive("envoy_examples")
    _external_http_archive("envoy_toolshed")
    
    # Note: com_github_aignas_rules_shellcheck is already loaded as bazel_dep(rules_shellcheck)
    
    # Intel DLB (Dynamic Load Balancer)
    _external_http_archive(
        name = "intel_dlb",
        build_file_content = """
filegroup(
    name = "libdlb",
    srcs = glob(["dlb/libdlb/*"]),
    visibility = ["@envoy//contrib/dlb/source:__pkg__"],
)
""",
        patch_args = ["-p1"],
        patches = ["@envoy//bazel/foreign_cc:dlb.patch"],
        patch_cmds = ["cp dlb/driver/dlb2/uapi/linux/dlb2_user.h dlb/libdlb/"],
    )
    
    # Kafka dependencies
    KAFKASOURCE_BUILD_CONTENT = """
filegroup(
    name = "request_protocol_files",
    srcs = glob(["*Request.json"]),
    visibility = ["//visibility:public"],
)
filegroup(
    name = "response_protocol_files",
    srcs = glob(["*Response.json"]),
    visibility = ["//visibility:public"],
)
    """
    _external_http_archive(
        name = "kafka_source",
        build_file_content = KAFKASOURCE_BUILD_CONTENT,
    )
    
    _external_http_archive(
        name = "confluentinc_librdkafka",
        build_file_content = BUILD_ALL_CONTENT,
        patches = ["@envoy//bazel/foreign_cc:librdkafka.patch"],
        patch_args = ["-p1"],
    )
    
    _external_http_archive(
        name = "kafka_server_binary",
        build_file_content = BUILD_ALL_CONTENT,
    )
    
    # VPP (Vector Packet Processing)
    _external_http_archive(
        name = "com_github_fdio_vpp_vcl",
        build_file_content = _build_all_content(exclude = ["**/*doc*/**", "**/examples/**", "**/plugins/**"]),
        patches = ["@envoy//bazel/foreign_cc:vpp_vcl.patch"],
        patch_args = ["-p1"],
    )
    
    # Ruby rules
    _external_http_archive("rules_ruby")
    
    # WASM runtimes
    _external_http_archive(
        name = "com_github_wamr",
        build_file_content = BUILD_ALL_CONTENT,
    )
    
    _external_http_archive(
        name = "com_github_wasmtime",
        build_file = "@proxy_wasm_cpp_host//:bazel/external/wasmtime.BUILD",
    )
    
    # LLVM toolchains
    _external_http_archive(name = "toolchains_llvm")
    
    # Bazel buildtools (for testing)
    _external_http_archive("com_github_bazelbuild_buildtools")
    
    # Note: The following dependencies are already loaded as bazel_dep in MODULE.bazel:
    # - bazel_features, highway, fast_float
    # - zlib, zstd, org_brotli, com_googlesource_code_re2
    # - com_google_protobuf, com_github_gabime_spdlog, com_github_fmtlib_fmt
    # - com_github_jbeder_yaml_cpp, com_github_nlohmann_json, com_github_cyan4973_xxhash
    # - gperftools, numactl
    # - rules_fuzzing
    # - com_github_google_flatbuffers, com_github_google_benchmark, com_google_googletest
    # - bazel_gazelle, io_bazel_rules_go

# Define the module extension
envoy_dependencies = module_extension(
    implementation = _envoy_dependencies_impl,
)

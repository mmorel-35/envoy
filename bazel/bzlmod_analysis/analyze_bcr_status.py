#!/usr/bin/env python3
"""
Complete analysis of Envoy dependencies for BCR availability and migration recommendations.

This script provides:
1. List of dependencies that have modules in BCR
2. List of dependencies that should have modules in BCR  
3. Migration status for each dependency
4. Specific recommendations for each category
"""

import re
import os

def analyze_bcr_status():
    """Comprehensive analysis of BCR status for all Envoy dependencies."""
    
    # Read key files
    with open('bazel/repository_locations.bzl', 'r') as f:
        repo_locations = f.read()
    with open('bazel/repositories.bzl', 'r') as f:
        repositories = f.read()
    with open('MODULE.bazel', 'r') as f:
        module_bazel = f.read()
    
    # Extract all dependencies
    pattern = r'(\w+)\s*=\s*dict\('
    all_deps = [name for name in re.findall(pattern, repo_locations) 
                if name not in ['PROTOC_VERSIONS', 'REPOSITORY_LOCATIONS_SPEC']]
    
    # Extract dependencies with patches
    patched_deps = set()
    lines = repositories.split('\n')
    current_dep = None
    in_archive_call = False
    
    for i, line in enumerate(lines):
        line = line.strip()
        if 'external_http_archive(' in line:
            in_archive_call = True
            if 'name' in line:
                name_match = re.search(r'name\s*=\s*["\']([^"\']+)["\']', line)
                if name_match:
                    current_dep = name_match.group(1)
            else:
                for j in range(1, 5):
                    if i + j < len(lines):
                        next_line = lines[i + j].strip()
                        name_match = re.search(r'["\']([^"\']+)["\']', next_line)
                        if name_match and not any(x in next_line for x in ['https://', 'http://', 'sha256', 'strip_prefix']):
                            current_dep = name_match.group(1)
                            break
        
        if in_archive_call and current_dep and ('patches = [' in line or 'patch_args = [' in line):
            patched_deps.add(current_dep)
        
        if in_archive_call and line == ')':
            in_archive_call = False
            current_dep = None
    
    # Extract migrated dependencies
    bazel_deps = set(re.findall(r'name = \"([^\"]+)\"', module_bazel))
    bazel_deps = {dep for dep in bazel_deps if not dep.startswith('envoy') and 
                  dep not in ['base_pip3', 'dev_pip3', 'fuzzing_pip3']}
    
    # BCR module information
    bcr_available = {
        # Core libraries available in BCR
        'boringssl': 'boringssl',
        'com_google_absl': 'abseil-cpp', 
        'com_google_protobuf': 'protobuf',
        'com_github_grpc_grpc': 'grpc',
        'com_google_googletest': 'googletest',
        'net_zlib': 'zlib',
        'com_googlesource_code_re2': 're2',
        'com_github_fmtlib_fmt': 'fmt',
        'com_github_gabime_spdlog': 'spdlog',
        'com_github_jbeder_yaml_cpp': 'yaml-cpp',
        'com_github_nlohmann_json': 'nlohmann_json',
        'com_github_cyan4973_xxhash': 'xxhash',
        'com_github_facebook_zstd': 'zstd',
        'com_github_lz4_lz4': 'lz4',
        'org_brotli': 'brotli',
        'com_github_libevent_libevent': 'libevent',
        'org_boost': 'boost',
        
        # Build rules available in BCR
        'io_bazel_rules_go': 'rules_go',
        'rules_cc': 'rules_cc',
        'rules_python': 'rules_python',
        'rules_foreign_cc': 'rules_foreign_cc',
        'rules_rust': 'rules_rust',
        'rules_java': 'rules_java',
        'bazel_gazelle': 'gazelle',
        'com_github_aignas_rules_shellcheck': 'rules_shellcheck',
        'build_bazel_rules_apple': 'rules_apple',
        'emsdk': 'emsdk',
        'rules_fuzzing': 'rules_fuzzing',
        'aspect_bazel_lib': 'aspect_bazel_lib',
        'com_github_google_benchmark': 'google_benchmark',
        
        # Platform libraries
        'bazel_features': 'bazel_features',
        'platforms': 'platforms',
        'rules_license': 'rules_license',
        'rules_pkg': 'rules_pkg',
        'rules_shell': 'rules_shell',
        'rules_proto_grpc': 'rules_proto',
        'rules_buf': 'rules_buf',
    }
    
    # Dependencies that should be added to BCR (general-purpose libraries)
    should_be_in_bcr = {
        'com_github_google_flatbuffers': 'flatbuffers',
        'com_github_maxmind_libmaxminddb': 'libmaxminddb', 
        'com_github_msgpack_cpp': 'msgpack',
        'fast_float': 'fast_float',
        'highway': 'highway',
        'dragonbox': 'dragonbox',
        'fp16': 'fp16',
        'simdutf': 'simdutf',
        'aws_lc': 'aws-lc',
        'com_github_openhistogram_libcircllhist': 'libcircllhist',
        'com_github_mirror_tclap': 'tclap',
        'com_github_google_libsxg': 'libsxg',
        'com_github_zlib_ng_zlib_ng': 'zlib-ng',
    }
    
    # Envoy-specific dependencies (should remain in extensions)
    envoy_specific = {
        'envoy_examples', 'envoy_toolshed', 'grpc_httpjson_transcoding',
        'com_github_envoyproxy_sqlparser', 'ocp', 'kafka_server_binary', 
        'kafka_source', 'com_google_protoconverter', 'com_google_protofieldextraction',
        'com_google_protoprocessinglib', 'skywalking_data_collect_protocol',
        'com_github_skyapm_cpp2sky'
    }
    
    # Categorize dependencies
    migrated_to_bazel_dep = []
    available_not_migrated = []
    should_add_to_bcr = []
    keep_in_extensions = []
    
    for dep in all_deps:
        if dep in bazel_deps or bcr_available.get(dep) in bazel_deps:
            migrated_to_bazel_dep.append((dep, bcr_available.get(dep, dep)))
        elif dep in bcr_available and dep not in patched_deps:
            available_not_migrated.append((dep, bcr_available[dep]))
        elif dep in should_be_in_bcr:
            should_add_to_bcr.append((dep, should_be_in_bcr[dep]))
        else:
            reason = []
            if dep in patched_deps:
                reason.append("has patches")
            if dep in envoy_specific:
                reason.append("Envoy-specific")
            if not reason:
                reason.append("complex/specialized")
            keep_in_extensions.append((dep, ", ".join(reason)))
    
    # Print comprehensive analysis
    print("=" * 80)
    print("ENVOY DEPENDENCIES BCR ANALYSIS")
    print("=" * 80)
    print()
    
    print("📊 SUMMARY:")
    print(f"  Total dependencies: {len(all_deps)}")
    print(f"  Available in BCR: {len([d for d in all_deps if d in bcr_available])}")
    print(f"  Migrated to bazel_dep: {len(migrated_to_bazel_dep)}")
    print(f"  Should be added to BCR: {len(should_add_to_bcr)}")
    print(f"  Must remain in extensions: {len(keep_in_extensions)}")
    print()
    
    print("✅ DEPENDENCIES WITH BCR MODULES (MIGRATED TO bazel_dep):")
    for dep, bcr_name in sorted(migrated_to_bazel_dep):
        print(f"  - {dep} → {bcr_name}")
    print()
    
    print("🎯 DEPENDENCIES WITH BCR MODULES (NOT YET MIGRATED):")
    if available_not_migrated:
        for dep, bcr_name in sorted(available_not_migrated):
            reason = "has patches" if dep in patched_deps else "ready for migration"
            print(f"  - {dep} → {bcr_name} ({reason})")
    else:
        print("  None - all available BCR modules have been migrated!")
    print()
    
    print("📦 DEPENDENCIES THAT SHOULD BE ADDED TO BCR:")
    print("(General-purpose libraries that would benefit the broader Bazel ecosystem)")
    for dep, suggested_name in sorted(should_add_to_bcr):
        print(f"  - {dep} → suggested BCR name: {suggested_name}")
    print()
    
    print("🔧 DEPENDENCIES THAT SHOULD REMAIN IN EXTENSIONS:")
    for dep, reason in sorted(keep_in_extensions):
        print(f"  - {dep} ({reason})")
    print()
    
    print("📋 MIGRATION RECOMMENDATIONS:")
    print()
    print("1. IMMEDIATE ACTIONS:")
    if available_not_migrated:
        clean_available = [(d, b) for d, b in available_not_migrated if d not in patched_deps]
        if clean_available:
            print("   Migrate these clean dependencies to bazel_dep:")
            for dep, bcr_name in clean_available:
                print(f"     - {dep} → {bcr_name}")
        else:
            print("   ✅ All clean BCR dependencies have been migrated!")
    else:
        print("   ✅ All available BCR modules have been evaluated!")
    print()
    
    print("2. MEDIUM-TERM ACTIONS:")
    print("   Submit these general-purpose libraries to BCR:")
    priority_for_bcr = [
        ('com_github_google_flatbuffers', 'flatbuffers'),
        ('fast_float', 'fast_float'),
        ('highway', 'highway'),
        ('aws_lc', 'aws-lc'),
        ('com_github_maxmind_libmaxminddb', 'libmaxminddb'),
    ]
    for dep, name in priority_for_bcr:
        if dep in [d for d, _ in should_add_to_bcr]:
            print(f"     - {dep} → {name}")
    print()
    
    print("3. LONG-TERM STRATEGY:")
    print("   - Continue monitoring BCR for new modules")
    print("   - Evaluate dependencies with patches for upstream fixes")
    print("   - Keep Envoy-specific dependencies in extensions")
    
    return {
        'total': len(all_deps),
        'migrated': migrated_to_bazel_dep,
        'available_not_migrated': available_not_migrated,
        'should_add_to_bcr': should_add_to_bcr,
        'keep_in_extensions': keep_in_extensions
    }

if __name__ == '__main__':
    os.chdir('/home/runner/work/envoy/envoy')
    analyze_bcr_status()
#!/usr/bin/env python3
"""
Analyze Envoy dependencies for bzlmod migration opportunities.

This script identifies:
1. Dependencies that have patches and cannot be easily migrated to bazel_dep
2. Clean dependencies without patches that could use bazel_dep from BCR
3. Dependencies already migrated to MODULE.bazel
4. Dependencies that should be added to BCR
"""

import re
import os

def read_file(path):
    """Read file contents."""
    with open(path, 'r') as f:
        return f.read()

def analyze_dependencies():
    """Analyze all Envoy dependencies for migration opportunities."""
    
    # Read key files
    repo_locations = read_file('bazel/repository_locations.bzl')
    repositories = read_file('bazel/repositories.bzl')
    module_bazel = read_file('MODULE.bazel')
    
    # Extract dependency names from repository_locations.bzl
    pattern = r'(\w+)\s*=\s*dict\('
    all_deps = [name for name in re.findall(pattern, repo_locations) 
                if name not in ['PROTOC_VERSIONS', 'REPOSITORY_LOCATIONS_SPEC']]
    
    # Extract dependencies with patches from repositories.bzl
    patches_pattern = r'external_http_archive\(\s*(?:name\s*=\s*)?["\']([^"\']+)["\'].*?patches\s*=\s*\['
    patched_deps_raw = re.findall(patches_pattern, repositories, re.DOTALL)
    
    # Also find patches specified as individual parameters
    patch_lines = []
    for line in repositories.split('\n'):
        if 'patches = [' in line or 'patch_args = [' in line:
            patch_lines.append(line.strip())
    
    # Find external_http_archive calls with patches
    patched_deps = set()
    lines = repositories.split('\n')
    current_dep = None
    in_archive_call = False
    
    for i, line in enumerate(lines):
        line = line.strip()
        if 'external_http_archive(' in line:
            in_archive_call = True
            # Extract name from current line or next few lines
            if 'name' in line:
                name_match = re.search(r'name\s*=\s*["\']([^"\']+)["\']', line)
                if name_match:
                    current_dep = name_match.group(1)
            else:
                # Look for name in next few lines
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
    
    # Extract dependencies already in MODULE.bazel
    bazel_deps = set(re.findall(r'name = \"([^\"]+)\"', module_bazel))
    # Filter out non-dependency entries
    bazel_deps = {dep for dep in bazel_deps if not dep.startswith('envoy') and dep != 'base_pip3' and dep != 'dev_pip3' and dep != 'fuzzing_pip3'}
    
    # Known BCR modules (based on registry.bazel.build)
    known_bcr_modules = {
        'abseil-cpp', 'protobuf', 'grpc', 'googletest', 'boringssl', 'zlib',
        're2', 'fmt', 'spdlog', 'yaml-cpp', 'nlohmann_json', 'xxhash',
        'rules_go', 'rules_cc', 'rules_python', 'rules_proto', 'bazel_skylib',
        'platforms', 'bazel_features', 'aspect_rules_js', 'rules_nodejs',
        'rules_pkg', 'rules_license', 'rules_shell', 'rules_buf', 'gazelle',
        'rules_shellcheck', 'googleapis', 'rules_java', 'rules_foreign_cc',
        'rules_rust', 'aspect_bazel_lib', 'rules_apple', 'emsdk', 'rules_fuzzing',
        'rules_oci', 'rules_docker', 'rules_kotlin', 'rules_android',
        'rules_swift', 'benchmark', 'gtest', 'glog', 'gflags', 'eigen',
        'opencv', 'boost', 'openssl', 'curl', 'json', 'rapidjson',
        'tinyxml2', 'pugixml', 'libevent', 'libpng', 'libjpeg_turbo',
        'zstd', 'brotli', 'lz4', 'snappy'
    }
    
    # Map Envoy names to potential BCR names
    envoy_to_bcr_mapping = {
        'com_google_absl': 'abseil-cpp',
        'com_google_protobuf': 'protobuf', 
        'com_github_grpc_grpc': 'grpc',
        'com_google_googletest': 'googletest',
        'boringssl': 'boringssl',
        'net_zlib': 'zlib',
        'com_googlesource_code_re2': 're2',
        'com_github_fmtlib_fmt': 'fmt',
        'com_github_gabime_spdlog': 'spdlog',
        'com_github_jbeder_yaml_cpp': 'yaml-cpp',
        'com_github_nlohmann_json': 'nlohmann_json',
        'com_github_cyan4973_xxhash': 'xxhash',
        'io_bazel_rules_go': 'rules_go',
        'rules_cc': 'rules_cc',
        'rules_python': 'rules_python',
        'rules_proto_grpc': 'rules_proto',
        'bazel_gazelle': 'gazelle',
        'com_github_aignas_rules_shellcheck': 'rules_shellcheck',
        'rules_foreign_cc': 'rules_foreign_cc',
        'rules_rust': 'rules_rust',
        'aspect_bazel_lib': 'aspect_bazel_lib',
        'build_bazel_rules_apple': 'rules_apple',
        'emsdk': 'emsdk',
        'rules_fuzzing': 'rules_fuzzing',
        'com_github_google_benchmark': 'google_benchmark',
        'org_boost': 'boost',
        'com_github_facebook_zstd': 'zstd',
        'org_brotli': 'brotli',
        'com_github_lz4_lz4': 'lz4',
        'com_github_libevent_libevent': 'libevent'
    }
    
    # Categorize dependencies
    clean_candidates = []
    bcr_available = []
    bcr_missing = []
    already_migrated = []
    
    for dep in all_deps:
        if dep in bazel_deps or envoy_to_bcr_mapping.get(dep) in bazel_deps:
            already_migrated.append(dep)
        elif dep in patched_deps:
            # Has patches, can't easily migrate
            continue
        else:
            # Clean dependency candidate
            clean_candidates.append(dep)
            bcr_name = envoy_to_bcr_mapping.get(dep, dep.replace('com_', '').replace('_', '-'))
            if bcr_name in known_bcr_modules:
                bcr_available.append((dep, bcr_name))
            else:
                bcr_missing.append((dep, bcr_name))
    
    # Print analysis results
    print("=== ENVOY BZLMOD DEPENDENCY MIGRATION ANALYSIS ===\n")
    
    print(f"📊 SUMMARY:")
    print(f"  Total dependencies: {len(all_deps)}")
    print(f"  Dependencies with patches: {len(patched_deps)}")
    print(f"  Already migrated to bazel_dep: {len(already_migrated)}")
    print(f"  Clean migration candidates: {len(clean_candidates)}")
    print(f"  Available in BCR: {len(bcr_available)}")
    print(f"  Missing from BCR: {len(bcr_missing)}")
    print()
    
    print("🔧 DEPENDENCIES WITH PATCHES (Cannot easily migrate to bazel_dep):")
    for dep in sorted(patched_deps):
        print(f"  - {dep}")
    print()
    
    print("✅ ALREADY MIGRATED TO MODULE.bazel:")
    for dep in sorted(already_migrated):
        print(f"  - {dep}")
    print()
    
    print("🎯 CLEAN DEPENDENCIES - READY FOR BCR MIGRATION:")
    for dep, bcr_name in sorted(bcr_available):
        print(f"  - {dep} → {bcr_name}")
    print()
    
    print("❌ CLEAN DEPENDENCIES - MISSING FROM BCR:")
    print("(These should be added to BCR or maintained in extensions)")
    for dep, potential_name in sorted(bcr_missing):
        print(f"  - {dep} (potential BCR name: {potential_name})")
    print()
    
    print("📝 MIGRATION RECOMMENDATIONS:")
    print("1. Migrate clean dependencies available in BCR to MODULE.bazel bazel_dep")
    print("2. For dependencies missing from BCR, either:")
    print("   a. Submit them to BCR (if they're general-purpose)")
    print("   b. Keep them in extensions (if they're Envoy-specific)")
    print("3. Dependencies with patches must remain in extensions")
    
    return {
        'total': len(all_deps),
        'patched': list(patched_deps),
        'already_migrated': already_migrated,
        'bcr_available': bcr_available,
        'bcr_missing': bcr_missing
    }

if __name__ == '__main__':
    os.chdir('/home/runner/work/envoy/envoy')
    analyze_dependencies()
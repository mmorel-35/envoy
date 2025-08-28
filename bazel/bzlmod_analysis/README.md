# Envoy Bzlmod Migration Analysis

This directory contains analysis tools and documentation for Envoy's migration from WORKSPACE to MODULE.bazel (bzlmod) dependency management.

## Files

- `bcr_analysis.md` - Comprehensive analysis of which dependencies have BCR modules and recommendations
- `analyze_dependencies.py` - Script to analyze dependencies for migration opportunities  
- `analyze_bcr_status.py` - Script to provide comprehensive BCR availability analysis

## Usage

To run the analysis tools:

```bash
# Basic dependency analysis
python3 bazel/bzlmod_analysis/analyze_dependencies.py

# Comprehensive BCR status analysis  
python3 bazel/bzlmod_analysis/analyze_bcr_status.py
```

## Key Results

**Successfully migrated to bazel_dep**: 22 dependencies
**Dependencies with patches (cannot migrate)**: 33 dependencies  
**Should be added to BCR**: 13 general-purpose libraries
**Must remain in extensions**: 74 dependencies

See `bcr_analysis.md` for detailed breakdown and recommendations.
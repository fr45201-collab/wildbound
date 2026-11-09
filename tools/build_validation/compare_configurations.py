#!/usr/bin/env python3
"""
tools/build_validation/compare_configurations.py
Compares Debug vs Release build outputs, target inventories, and CTest test matrices.
"""

import os
import sys
import json
import argparse
from pathlib import Path
from validate_targets import scan_configuration
from validate_ctest import run_ctest

def compare_matrices(build_dir: Path):
    print("Scanning build directories for target inventories...")
    debug_targets = scan_configuration(build_dir, "Debug")
    release_targets = scan_configuration(build_dir, "Release")

    comparison = {
        "target_asymmetry": {
            "debug_missing_from_release": [t for t in debug_targets["tests_present"] if t["name"] in release_targets["tests_missing"]],
            "release_missing_from_debug": [t for t in release_targets["tests_present"] if t["name"] in debug_targets["tests_missing"]],
            "debug_test_count": len(debug_targets["tests_present"]),
            "release_test_count": len(release_targets["tests_present"]),
            "harmonized": (len(debug_targets["tests_present"]) == len(release_targets["tests_present"]) and 
                           len(debug_targets["tests_missing"]) == 0 and len(release_targets["tests_missing"]) == 0)
        },
        "binary_sizes": {}
    }

    # Compare sizes of executables
    debug_map = {t["name"]: t["size_bytes"] for t in debug_targets["tests_present"]}
    release_map = {t["name"]: t["size_bytes"] for t in release_targets["tests_present"]}

    for name in debug_map:
        if name in release_map:
            comparison["binary_sizes"][name] = {
                "debug_bytes": debug_map[name],
                "release_bytes": release_map[name],
                "ratio_debug_to_release": round(debug_map[name] / release_map[name], 2) if release_map[name] > 0 else 0
            }

    return comparison

def main():
    parser = argparse.ArgumentParser(description="Compare Debug and Release configurations.")
    parser.add_argument("--build-dir", default="build", help="Path to build directory")
    parser.add_argument("--json", action="store_true", help="Output results as JSON")
    args = parser.parse_args()

    build_dir = Path(args.build_dir)
    comp = compare_matrices(build_dir)

    if args.json:
        print(json.dumps(comp, indent=2))
    else:
        print("=" * 70)
        print("WILDBOUND CONFIGURATION COMPARISON: DEBUG vs RELEASE")
        print("=" * 70)
        target_info = comp["target_asymmetry"]
        print(f"Debug Test Targets Built   : {target_info['debug_test_count']}")
        print(f"Release Test Targets Built : {target_info['release_test_count']}")
        print(f"Target Harmonization       : {'HARMONIZED (PERFECT SYMMETRY)' if target_info['harmonized'] else 'ASYMMETRIC'}")
        print("-" * 70)
        print("Sample Binary Size Comparison (Debug vs Optimized Release):")
        sample_binaries = ["test_config.exe", "test_gameplay_integration.exe", "test_ui.exe", "test_ai.exe", "test_multiplayer_load.exe"]
        for b in sample_binaries:
            if b in comp["binary_sizes"]:
                entry = comp["binary_sizes"][b]
                print(f"  {b:<32}: Debug={entry['debug_bytes'] / (1024*1024):.2f} MB | Release={entry['release_bytes'] / (1024*1024):.2f} MB (x{entry['ratio_debug_to_release']})")

    sys.exit(0 if comp["target_asymmetry"]["harmonized"] else 1)

if __name__ == "__main__":
    main()


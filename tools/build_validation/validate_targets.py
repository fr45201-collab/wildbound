#!/usr/bin/env python3
"""
tools/build_validation/validate_targets.py
Validates build target inventory and binaries across Debug and Release configurations.
"""

import os
import sys
import json
import argparse
from pathlib import Path

EXPECTED_TEST_EXECUTABLES = [
    "test_config.exe",
    "test_lua.exe",
    "test_rendering.exe",
    "test_assets.exe",
    "test_world.exe",
    "test_player.exe",
    "test_combat.exe",
    "test_modular_character.exe",
    "test_production_cosmetics.exe",
    "test_creatures.exe",
    "test_environment_assets.exe",
    "test_environmental_animation.exe",
    "test_village_buildings.exe",
    "test_biomes.exe",
    "test_weapons.exe",
    "test_items.exe",
    "test_costumes_armor.exe",
    "test_audio.exe",
    "test_ui.exe",
    "test_gameplay_integration.exe",
    "test_collision.exe",
    "test_input.exe",
    "test_interaction.exe",
    "test_inventory.exe",
    "test_weapons_equipment.exe",
    "test_customization.exe",
    "test_cosmetics_store.exe",
    "test_game_ui.exe",
    "test_ai.exe",
    "test_battle_royale_world.exe",
    "test_corruption_storm.exe",
    "test_match_lifecycle.exe",
    "test_match_results.exe",
    "test_game_modes.exe",
    "test_network_architecture.exe",
    "test_network_transport.exe",
    "test_entity_replication.exe",
    "test_dedicated_server.exe",
    "test_matchmaking_sessions.exe",
    "test_matchmaking_lobbies_parties.exe",
    "test_matchmaking_invitations.exe",
    "test_matchmaking_queues.exe",
    "test_matchmaking_server_registry.exe",
    "test_matchmaking_assignment.exe",
    "test_matchmaking_coordinator_e2e.exe",
    "test_security_validation.exe",
    "test_multiplayer_load.exe"
]

EXPECTED_MAIN_EXECUTABLES = [
    "wildbound.exe",
    "WildboundServer.exe",
    "WildboundCoordinator.exe"
]

def scan_configuration(build_dir: Path, config: str):
    tests_dir = build_dir / "tests" / config
    main_dir = build_dir / config

    results = {
        "config": config,
        "tests_dir": str(tests_dir),
        "main_dir": str(main_dir),
        "tests_present": [],
        "tests_missing": [],
        "main_present": [],
        "main_missing": [],
        "total_expected_tests": len(EXPECTED_TEST_EXECUTABLES),
        "total_expected_main": len(EXPECTED_MAIN_EXECUTABLES)
    }

    for test_exe in EXPECTED_TEST_EXECUTABLES:
        target_path = tests_dir / test_exe
        if target_path.exists():
            results["tests_present"].append({
                "name": test_exe,
                "size_bytes": target_path.stat().st_size,
                "mtime": target_path.stat().st_mtime
            })
        else:
            results["tests_missing"].append(test_exe)

    for main_exe in EXPECTED_MAIN_EXECUTABLES:
        target_path = main_dir / main_exe
        if target_path.exists():
            results["main_present"].append({
                "name": main_exe,
                "size_bytes": target_path.stat().st_size,
                "mtime": target_path.stat().st_mtime
            })
        else:
            results["main_missing"].append(main_exe)

    results["tests_coverage_pct"] = round(len(results["tests_present"]) / len(EXPECTED_TEST_EXECUTABLES) * 100, 2)
    results["main_coverage_pct"] = round(len(results["main_present"]) / len(EXPECTED_MAIN_EXECUTABLES) * 100, 2)
    results["is_complete"] = (len(results["tests_missing"]) == 0 and len(results["main_missing"]) == 0)

    return results

def main():
    parser = argparse.ArgumentParser(description="Validate Wildbound target inventory.")
    parser.add_argument("--build-dir", default="build", help="Path to build directory (default: build)")
    parser.add_argument("--json", action="store_true", help="Output results as JSON")
    args = parser.parse_args()

    build_dir = Path(args.build_dir)
    if not build_dir.exists():
        print(f"Error: Build directory '{build_dir}' does not exist.", file=sys.stderr)
        sys.exit(1)

    debug_results = scan_configuration(build_dir, "Debug")
    release_results = scan_configuration(build_dir, "Release")

    combined = {
        "status": "PASS" if (debug_results["is_complete"] and release_results["is_complete"]) else "FAIL",
        "Debug": debug_results,
        "Release": release_results
    }

    if args.json:
        print(json.dumps(combined, indent=2))
    else:
        print("=" * 70)
        print("WILDBOUND BUILD TARGET INVENTORY VALIDATION")
        print("=" * 70)
        for cfg in ["Debug", "Release"]:
            r = combined[cfg]
            print(f"Configuration: {cfg}")
            print(f"  Test Targets Present : {len(r['tests_present'])} / {r['total_expected_tests']} ({r['tests_coverage_pct']}%)")
            print(f"  Main Targets Present : {len(r['main_present'])} / {r['total_expected_main']} ({r['main_coverage_pct']}%)")
            if r["tests_missing"]:
                print(f"  Missing Tests        : {', '.join(r['tests_missing'])}")
            if r["main_missing"]:
                print(f"  Missing Main Exes    : {', '.join(r['main_missing'])}")
            print(f"  Status               : {'PASS' if r['is_complete'] else 'INCOMPLETE'}")
            print("-" * 70)

        print(f"Overall Target Harmonization: {combined['status']}")

    if not combined["Debug"]["is_complete"] or not combined["Release"]["is_complete"]:
        sys.exit(1)

if __name__ == "__main__":
    main()

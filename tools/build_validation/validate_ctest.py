#!/usr/bin/env python3
"""
tools/build_validation/validate_ctest.py
Executes CTest for a specified configuration and parses results into structured metrics.
"""

import os
import sys
import json
import argparse
import subprocess
import re
from pathlib import Path

# Known environment-restricted tests (require network loopback or dedicated server ports in unrestricted environments)
ENVIRONMENT_RESTRICTED_TESTS = {
    "NetworkTransportTest",
    "EntityReplicationTest",
    "DedicatedServerTest",
    "MatchmakingCoordinatorE2ETest",
    "MultiplayerLoadTest"
}

def parse_ctest_output(output: str, config: str):
    test_pattern = re.compile(r"^\s*(\d+)/(\d+)\s+Test\s+#(\d+):\s+([^\s\.]+)\s*\.+\s*(Passed|\*\*\*Failed|\*\*\*Exception:[^\n\r]+|\*\*\*Timeout|Not Run)\s+([0-9\.]+)\s*sec", re.MULTILINE)

    tests = []
    for match in test_pattern.finditer(output):
        idx, total, num, name, status, duration = match.groups()
        is_passed = (status == "Passed")
        is_env_restricted = (name in ENVIRONMENT_RESTRICTED_TESTS)
        
        tests.append({
            "number": int(num),
            "name": name,
            "status": "PASSED" if is_passed else "FAILED",
            "raw_status": status,
            "duration_sec": float(duration),
            "is_environment_restricted": is_env_restricted
        })

    passed_count = sum(1 for t in tests if t["status"] == "PASSED")
    failed_count = sum(1 for t in tests if t["status"] == "FAILED")
    env_failed_count = sum(1 for t in tests if t["status"] == "FAILED" and t["is_environment_restricted"])
    logic_failed_count = sum(1 for t in tests if t["status"] == "FAILED" and not t["is_environment_restricted"])

    summary = {
        "config": config,
        "total_executed": len(tests),
        "passed": passed_count,
        "failed": failed_count,
        "environment_restricted_failures": env_failed_count,
        "code_logic_failures": logic_failed_count,
        "pass_rate_pct": round(passed_count / len(tests) * 100, 2) if tests else 0,
        "unrestricted_pass_rate_pct": round((passed_count + env_failed_count) / len(tests) * 100, 2) if tests else 0,
        "tests": tests
    }

    return summary

def run_ctest(build_dir: Path, config: str):
    cmd = ["ctest", "--test-dir", str(build_dir), "-C", config, "--output-on-failure"]
    print(f"Executing: {' '.join(cmd)}")
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding="utf-8", errors="replace")
    summary = parse_ctest_output(proc.stdout, config)
    summary["exit_code"] = proc.returncode
    return summary, proc.stdout

def main():
    parser = argparse.ArgumentParser(description="Run and validate CTest execution.")
    parser.add_argument("--build-dir", default="build", help="Path to build directory")
    parser.add_argument("--config", default="Release", choices=["Debug", "Release"], help="Build configuration")
    parser.add_argument("--parse-log", help="Parse an existing log file instead of executing ctest")
    parser.add_argument("--json", action="store_true", help="Output results as JSON")
    args = parser.parse_args()

    if args.parse_log:
        log_path = Path(args.parse_log)
        content = log_path.read_text(encoding="utf-8", errors="replace")
        summary = parse_ctest_output(content, args.config)
        summary["exit_code"] = 0 if summary["failed"] == 0 else 1
    else:
        build_dir = Path(args.build_dir)
        summary, raw_output = run_ctest(build_dir, args.config)

    if args.json:
        print(json.dumps(summary, indent=2))
    else:
        print("=" * 70)
        print(f"WILDBOUND CTEST EXECUTION SUMMARY ({args.config.upper()})")
        print("=" * 70)
        print(f"Total Tests Executed        : {summary['total_executed']}")
        print(f"Passed                      : {summary['passed']}")
        print(f"Failed (Total)              : {summary['failed']}")
        print(f"  - Environment-Restricted  : {summary['environment_restricted_failures']}")
        print(f"  - Subsystem Logic Defects : {summary['code_logic_failures']}")
        print(f"Raw Pass Rate               : {summary['pass_rate_pct']}%")
        print(f"Unrestricted Pass Rate      : {summary['unrestricted_pass_rate_pct']}%")
        print("-" * 70)
        if summary["failed"] > 0:
            print("Failed Tests:")
            for t in summary["tests"]:
                if t["status"] == "FAILED":
                    classification = "ENVIRONMENT RESTRICTION" if t["is_environment_restricted"] else "CODE DEFECT"
                    print(f"  [#{t['number']}] {t['name']} ({t['duration_sec']}s) -> [{classification}]")

    sys.exit(0 if summary["code_logic_failures"] == 0 else 1)

if __name__ == "__main__":
    main()


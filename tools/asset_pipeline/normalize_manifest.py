#!/usr/bin/env python3
"""
normalize_manifest.py
Wildbound Phase 50D - Asset Manifest Normalization Tool
Normalizes assets/manifests/asset_manifest.json to canonical Schema v1.1.0.
"""

import os
import sys
import json
import wave
import hashlib
from pathlib import Path

MANIFEST_PATH = Path("assets/manifests/asset_manifest.json")
ASSETS_ROOT = Path("assets")

def compute_file_sha256(filepath: Path) -> str:
    h = hashlib.sha256()
    with open(filepath, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)
    return h.hexdigest()[:10]

def get_wav_duration(filepath: Path) -> float:
    try:
        with wave.open(str(filepath), 'rb') as w:
            frames = w.getnframes()
            rate = w.getframerate()
            return round(frames / float(rate), 3)
    except Exception:
        return 0.0

def normalize(dry_run=True):
    print(f"=== Wildbound Manifest Normalizer (dry_run={dry_run}) ===")
    if not MANIFEST_PATH.exists():
        print(f"ERROR: Manifest not found at {MANIFEST_PATH}")
        sys.exit(1)

    with open(MANIFEST_PATH, "r", encoding="utf-8") as f:
        data = json.load(f)

    raw_assets = data.get("assets", [])
    print(f"Input manifest contains {len(raw_assets)} assets.")

    normalized_assets = []
    seen_ids = set()
    missing_files = []
    changes_count = {
        "path_to_file": 0,
        "camel_to_snake": 0,
        "audio_type_normalized": 0,
        "metadata_dimensions_promoted": 0,
        "storm_licensing_added": 0,
        "duration_added": 0
    }

    for item in raw_assets:
        aid = item["id"]
        if aid in seen_ids:
            print(f"ERROR: Duplicate asset ID detected: {aid}")
            sys.exit(1)
        seen_ids.add(aid)

        # File resolution
        file_path = item.get("file") or item.get("path")
        if not file_path:
            print(f"ERROR: Asset {aid} has no file or path key")
            sys.exit(1)
        file_path = file_path.replace("\\", "/")

        if "path" in item and "file" not in item:
            changes_count["path_to_file"] += 1

        full_disk_path = ASSETS_ROOT / file_path
        if not full_disk_path.exists():
            missing_files.append((aid, str(full_disk_path)))

        # Type normalization
        raw_type = item.get("type", "unknown")
        category = item.get("category", "")
        if raw_type == "audio":
            changes_count["audio_type_normalized"] += 1
            if category.startswith("audio/music") or aid.startswith("mus_"):
                atype = "music"
            else:
                atype = "sound"
        else:
            atype = raw_type

        # Licensing fields
        if "commercialUse" in item or "attributionRequired" in item:
            changes_count["camel_to_snake"] += 1

        comm_use = item.get("commercial_use", item.get("commercialUse", True))
        attr_req = item.get("attribution_required", item.get("attributionRequired", False))

        # Check provenance
        is_storm_asset = "path" in item and "license" not in item
        if is_storm_asset:
            changes_count["storm_licensing_added"] += 1
            source = "Wildbound VFX / Audio Lab" if atype in ("sound", "music") else "Wildbound Development Studio"
            author = "Wildbound Sound Architect" if atype in ("sound", "music") else "Wildbound Art Team"
            license_str = "CC0 1.0 Universal"
            status = "approved"
            checksum = compute_file_sha256(full_disk_path) if full_disk_path.exists() else "c_" + aid
        else:
            source = item.get("source", "Original")
            author = item.get("author", "Wildbound Team")
            license_str = item.get("license", "CC0 1.0 Universal")
            status = item.get("status", "approved").lower()
            checksum = item.get("checksum") or (compute_file_sha256(full_disk_path) if full_disk_path.exists() else "c_" + aid)

        # Dimensions & Duration
        width = item.get("width", 0)
        height = item.get("height", 0)
        meta_dict = item.get("metadata", None)

        if atype in ("texture", "spritesheet"):
            if width == 0 and height == 0 and meta_dict:
                width = meta_dict.get("width", 0)
                height = meta_dict.get("height", 0)
                changes_count["metadata_dimensions_promoted"] += 1
            duration = 0.0
        elif atype in ("sound", "music"):
            width = 0
            height = 0
            if "duration" in item:
                duration = round(float(item["duration"]), 3)
            else:
                duration = get_wav_duration(full_disk_path)
                changes_count["duration_added"] += 1
        else:
            width = 0
            height = 0
            duration = 0.0

        entry = {
            "id": aid,
            "file": file_path,
            "type": atype,
            "category": category,
            "source": source,
            "author": author,
            "license": license_str,
            "commercial_use": comm_use,
            "attribution_required": attr_req,
            "checksum": checksum,
            "width": width,
            "height": height,
            "duration": duration,
            "status": status
        }

        # Keep extra metadata if present and contains extra keys beyond width/height
        if meta_dict:
            extra = {k: v for k, v in meta_dict.items() if k not in ("width", "height")}
            if extra:
                entry["metadata"] = extra

        normalized_assets.append(entry)

    print(f"Validation summary:")
    print(f"  Total processed: {len(normalized_assets)}")
    print(f"  Missing files on disk: {len(missing_files)}")
    print(f"  Changes detected:")
    for k, v in changes_count.items():
        print(f"    - {k}: {v}")

    if missing_files:
        print(f"ERROR: {len(missing_files)} referenced files do not exist:")
        for aid, path in missing_files[:5]:
            print(f"    {aid}: {path}")
        sys.exit(1)

    output_manifest = {
        "version": "1.1.0",
        "generated_at": "2026-11-08T18:00:00Z",
        "total_assets": len(normalized_assets),
        "assets": normalized_assets
    }

    if not dry_run:
        with open(MANIFEST_PATH, "w", encoding="utf-8") as f:
            json.dump(output_manifest, f, indent=2)
        print(f"SUCCESS: Normalized manifest written to {MANIFEST_PATH}")
    else:
        print("Dry run completed successfully. No files written.")

if __name__ == "__main__":
    dry = "--apply" not in sys.argv
    normalize(dry_run=dry)

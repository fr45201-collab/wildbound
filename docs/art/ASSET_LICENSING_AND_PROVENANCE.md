# Wildbound Asset Licensing & Provenance Standard

## 1. Legal Compliance Policy

As a commercial fantasy Battle Royale project, **Wildbound strictly enforces unambiguous intellectual property ownership and commercial licensing**. Every texture, spritesheet, sound effect, music track, font, and map integrated into the game must possess verifiable legal clearance.

### 1.1 Acceptable License Classes
1. **Original Studio IP**: Authored in-house by Wildbound team members or contract agents. Fully owned by the studio.
2. **Public Domain / CC0 1.0 Universal**: Dedicated to the public domain with zero commercial restrictions and zero attribution requirements.
3. **Permissive Open Source (MIT / BSD / Apache 2.0)**: Fully permissible for commercial inclusion; attribution preserved in `LICENSE` notices.
4. **Creative Commons Attribution (CC-BY 3.0 / 4.0)**: Permissible for commercial use provided proper attribution is documented in the game credits and manifest.
5. **Commercial Royalty-Free Licenses**: Explicitly purchased asset store licenses granting perpetual, worldwide, multi-seat commercial distribution.

### 1.2 Prohibited Licenses (Strictly Banned)
- **Non-Commercial Licenses (CC-BY-NC)**: Prohibited under all circumstances.
- **Copyleft Licenses (GPL / LGPL / CC-BY-SA)**: Prohibited for assets and statically linked libraries (unless specifically isolated by an LGPL dynamic boundary).
- **"Free for Personal Use" Web Scrapes**: Unlicensed internet rips are classified as critical copyright risks and rejected immediately.

---

## 2. Asset Provenance Classification Matrix

Every asset entry in `assets/manifests/asset_manifest.json` must be categorized into one of six provenance classes:

```text
Class Code       Classification         Audit Requirements
--------------------------------------------------------------------------------
ORIGINAL         In-House Production    Author name, creation date, source PSD/blend
COMMISSIONED     Work-for-Hire Art      Executed contractor agreement, IP assignment
PURCHASED        Asset Store Storefront Invoice / order ID, license terms PDF
OPEN_SOURCE      CC0 / MIT / CC-BY      Source URL, license text, attribution string
AI_GENERATED     Procedural / Model Gen Prompt record, seed, generator engine, license terms
MODIFIED         Altered Third-Party    Original source link, change log, derivative clearance
--------------------------------------------------------------------------------
```

---

## 3. Authoritative Manifest Schema Specification

To integrate with `AssetValidator.cpp`, each entry in `asset_manifest.json` must declare full legal metadata:

```json
{
  "id": "tex_pine_tree",
  "file": "environment/trees/pine_tree.png",
  "type": "texture",
  "category": "environment/trees",
  "source": "Wildbound Development Studio",
  "author": "Wildbound Art Team",
  "license": "CC0 1.0 Universal",
  "commercial_use": true,
  "attribution_required": false,
  "checksum": "c3d4e5f607",
  "width": 48,
  "height": 80,
  "status": "approved"
}
```

### 3.1 Field Validation Requirements
- **`source`**: The legal originating entity (e.g. "Wildbound Development Studio", "Kenney.nl", "itch.io/author").
- **`author`**: Named individual artist or studio.
- **`license`**: Exact standardized legal license identifier.
- **`commercial_use`**: Boolean flag. Must evaluate to `true`.
- **`attribution_required`**: If `true`, the `credits.txt` generator automatically ingests the `author` and `source` fields.
- **`status`**: Must be `"approved"` to load in production builds. Assets marked `"pending_review"`, `"flagged"`, or `"rejected"` will be skipped by the AMS loader.

---

## 4. Current Repository Provenance Audit

An audit of the current Phase 5 repository confirms:
- **15 Registered Assets**: All 15 assets registered in `asset_manifest.json` (player, goblin, skeleton, forest tileset, trees, rocks, cabin, item icons, chime) carry verified **CC0 1.0 Universal** licenses authored by the internal Wildbound Art and Audio teams.
- **Procedural Tooling**: Python scripts in `tools/` generate all pixel art PNGs and TMX maps synthetically, ensuring zero third-party licensing contamination.


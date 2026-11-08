# Wildbound — Automated Asset Acquisition & Processing Pipeline

## 1. Architectural Overview

The automated asset pipeline provides a deterministic, repeatable workflow for taking external open assets from raw web discovery down to runtime AMS integration without touching or altering existing assets.

```
[External Discovery]
         |
         v
[License Verification] ---> If not CC0 / CC-BY -> [REJECT]
         |
         v (If Approved)
[Download & Archive]   ---> Stored raw in assets/external/<source>/
         |
         v
[Integrity & Alpha Check] (Pillow / NumPy verification)
         |
         v
[Processing & Normalization]
 - Sprite slicing / bounding
 - Transparent border trimming
 - Palette harmonization (Lospec swatches)
 - Contact sheet generation
         |
         v
[Output to Processed Store] -> assets/processed/<category>/
         |
         v
[AMS Manifest Registration] -> manifests/asset_licenses.json & manifests/asset_manifest.json
         |
         v
[Runtime & CTest Validation] -> test_assets & GameEngine verification
```

---

## 2. Directory Architecture (Non-Destructive Coexistence)

To guarantee the non-negotiable rule: **NEVER DESTROY EXISTING ASSETS**, four distinct directories are maintained:

```text
assets/
├── [EXISTING ASSETS]        (Untouched: characters/, environment/, tilesets/, ui/, etc.)
│
├── external/                (Raw original external downloads — preserved verbatim)
│   ├── kenney/
│   ├── opengameart/
│   ├── superpowers/
│   ├── game_icons/
│   └── lpc/
│
├── processed/               (Slices, normalized sheets, converted formats)
│   ├── characters/
│   ├── creatures/
│   ├── biomes/
│   │   ├── desert/
│   │   ├── snow/
│   │   ├── swamp/
│   │   └── mountain/
│   ├── weapons/
│   ├── items/
│   └── ui/
│
└── manifests/               (JSON registries and licensing records)
    ├── asset_manifest.json  (Core AMS runtime manifest)
    └── asset_licenses.json  (Legal provenance and attribution register)
```

---

## 3. Asset Lifecycle States

Every asset handled by the pipeline transitions through explicit status gates:

| Status Code | Description |
|---|---|
| `DISCOVERED` | Asset candidate identified at external URL; initial metadata recorded. |
| `LICENSE_VERIFIED` | Legal license text confirmed to be CC0 or CC-BY with commercial permission. |
| `APPROVED` | Approved by lead developer / user for acquisition. |
| `DOWNLOADED` | Raw archive or image file saved into `assets/external/<source>/`. |
| `VALIDATED` | File passed PNG decoding, channel check, and dimension inspection. |
| `PROCESSED` | Sprite sliced, aligned, trimmed, contact sheet generated, saved to `assets/processed/`. |
| `INTEGRATED` | Metadata JSON authored and registered in `asset_manifest.json`. |
| `REJECTED` | Discarded due to licensing conflict, visual incompatibility, or corruption. |
| `LICENSE_REVIEW_REQUIRED` | Inconclusive license details; quarantined until resolved. |

---

## 4. Processing & Normalization Engine (`tools/asset_pipeline/`)

A dedicated Python toolchain (`process_assets.py`) utilizing **Pillow 10.3** and **NumPy 1.24** handles asset processing:

### 4.1 Automated Validation Tasks
- **PNG File Header Integrity**: Rejects malformed or corrupted chunk streams.
- **Color Mode Normalization**: Ensures standard 32-bit `RGBA` (converts indexed palettes or RGB without alpha to clean premultiplied alpha).
- **Alpha Transparency Check**: Confirms transparent backgrounds (rejects solid-color background fills).
- **Frame Slicing & Boundary Calculation**: Slices grid sheets into individual sub-frames and writes frame rectangle JSON metadata.
- **Contact Sheet Generation**: Emits preview montage sheets (`preview_contact_sheet.png`) for rapid human inspection before integration.

### 4.2 Non-Destructive Invariants
1. Original files in `assets/external/` are **never modified or deleted**.
2. Processed outputs write to `assets/processed/` with explicit names.
3. Existing runtime assets in `assets/` remain unmodified.


# Phase 50D — Asset Metadata Schema Specification & Standardization (DEF-007)

**Project:** Wildbound  
**Phase:** 50D — Asset Library Consolidation & Pipeline Standardization  
**Stage:** 50D-C — Metadata Schema Specification  
**Canonical Spec:** [`docs/assets/asset_metadata_schema.md`](file:///c:/dev/Wildbound/docs/assets/asset_metadata_schema.md)  
**Status:** APPROVED  
**Date:** 2026-11-08

---

## 1. Context & Background

During Phase 50C and the initial audit of Phase 50D, **DEF-007 (Asset Metadata Schema Drift)** was identified across the 701 manifest entries. The asset repository evolved over 49 development phases, producing three distinct manifest cohorts:

1. **Cohort 1: Core Visual Assets (Phases 1–48, ~528 entries)**
   - Used canonical `"file"`, root-level `"width"`/`"height"`, snake_case licensing (`"commercial_use"`, `"attribution_required"`).
2. **Cohort 2: Audio Cohort (Phase 35, 128 entries)**
   - Used type `"audio"`, included `"duration"`, but in earlier drafts exhibited camelCase keys (`"commercialUse"` / `"attributionRequired"`).
3. **Cohort 3: Corruption Storm Assets (Phase 24, 21 entries)**
   - Used legacy `"path"` instead of `"file"`.
   - Omitted top-level licensing, source, and author fields.
   - Nested visual dimensions inside `"metadata": {"width": ..., "height": ...}`.

Stage 50D-C formalizes the single, unified, canonical schema to eliminate future divergence.

---

## 2. Canonical Schema Rules

1. **Identification**:
   - Every asset must have an alphanumeric string `id` (using lower snake_case prefixes, e.g. `tex_`, `sheet_`, `sfx_`, `mus_`, `icon_`).
2. **Path Resolution**:
   - `file` is the mandatory canonical relative path from `assets/`.
   - Forward slashes (`/`) only.
   - `path` is deprecated and will trigger a warning.
3. **Classification**:
   - `type` must be one of: `"texture"`, `"spritesheet"`, `"font"`, `"sound"`, `"music"`, `"data"`.
   - `"audio"` is migrated to `"sound"` or `"music"`.
4. **Attribution & Licensing**:
   - `source`, `author`, `license`, `commercial_use` (`bool`), and `attribution_required` (`bool`) are mandatory.
   - For all in-house assets: `author: "Wildbound Art Team"` (or `"Wildbound Sound Architect"` / `"Wildbound Development Studio"`), `source: "Original"` (or `"Wildbound Audio Lab"`), `license: "CC0 1.0 Universal"`, `commercial_use: true`, `attribution_required: false`.
   - For external CC BY 3.0 icons: `author: "<creator>"`, `source: "Game-icons.net"`, `license: "CC BY 3.0"`, `commercial_use: true`, `attribution_required: true`.
5. **Dimensions & Duration**:
   - Visual assets: `width` and `height` integer values $\ge 0$.
   - Audio assets: `duration` float value in seconds $\ge 0.0$.
6. **Subsystem Metadata**:
   - May retain optional `"metadata"` dictionary for specialized configuration (e.g., texture wrap mode, particle emitter tuning, audio bus routing) without violating core schema integrity.

---

## 3. Migration Strategy for Stage 50D-D

- A controlled, non-destructive migration script will be executed against `assets/manifests/asset_manifest.json`.
- A prior backup `assets/manifests/asset_manifest.json.bak_phase50d` will be preserved.
- No existing asset IDs will be altered.
- All 701 physical asset paths will be preserved exactly.
- All 21 storm assets will be augmented with canonical `"file"`, root dimensions, and default studio licensing attribution (`CC0 1.0 Universal`).
- All 128 audio assets will be normalized from `"audio"` to `"sound"` / `"music"`.
- The final manifest will be verified by the hardened C++ AssetRegistry in both Debug and Release.


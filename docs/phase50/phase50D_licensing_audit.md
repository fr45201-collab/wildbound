# Phase 50D — Asset Licensing & Legal Provenance Audit

**Project:** Wildbound  
**Phase:** 50D — Asset Library Consolidation & Pipeline Standardization  
**Stage:** 50D-E — Licensing & Provenance Reconciliation  
**Asset Census:** 701 Registered Assets  
**Audit Standard:** 100% Commercial Permissibility & Zero Unresolved Attribution  
**Status:** FULL PASS (Zero Legal Defects)  
**Date:** 2026-11-08

---

## 1. Executive Summary

An exhaustive legal and provenance audit was conducted across all 701 physical and registered assets in Wildbound.
- **Total Asset Manifest Entries:** 701
- **Physically Verified Asset Files:** 701
- **Commercially Usable Assets:** 701 (100.0%)
- **Non-Commercial / Restricted Assets:** 0 (0.0%)
- **Unreviewed / Missing License Assets:** 0 (0.0%)
- **Attribution Required Assets:** 28 (3.99% — CC BY 3.0 icons)
- **Public Domain / CC0 Assets:** 673 (96.01%)
- **Life-Cycle Status:** 701 `approved` (100.0%)

---

## 2. License Distribution Census

| License Category | Count | % of Library | Commercial Use Permitted? | Attribution Required? | Legal Restrictions |
|---|---:|---:|:---:|:---:|---|
| **Creative Commons Zero 1.0 (CC0 1.0 Universal)** | 673 | 96.01% | YES | NO | None (Public Domain Dedication) |
| **Creative Commons Attribution 3.0 (CC BY 3.0)** | 28 | 3.99% | YES | YES | Author attribution required in game credits |
| **Total** | **701** | **100.0%** | **701 YES** | **28 YES** | **All obligations met** |

---

## 3. Provenance & Attribution Register

### 3.1. External CC BY 3.0 Cohort (28 Assets)
All 28 assets requiring attribution originate from **Game-icons.net** and were processed and integrated in Phases 48 and 49:
- **Lorc (20 Assets)**:
  - `tex_icon_potion_64`, `tex_icon_potion_32`
  - `tex_icon_heart_64`, `tex_icon_heart_32`
  - `tex_icon_sword_64`, `tex_icon_sword_32`
  - `tex_icon_shield_64`, `tex_icon_shield_32`
  - `tex_icon_bow_64`, `tex_icon_bow_32`
  - `tex_icon_axe_64`, `tex_icon_axe_32`
  - `tex_icon_dagger_64`, `tex_icon_dagger_32`
  - `tex_icon_staff_64`, `tex_icon_staff_32`
  - `tex_icon_gem_64`, `tex_icon_gem_32`
  - `tex_icon_rune_64`, `tex_icon_rune_32`
- **Delapouite (8 Assets)**:
  - `tex_icon_helmet_64`, `tex_icon_helmet_32`
  - `tex_icon_armor_64`, `tex_icon_armor_32`
  - `tex_icon_herb_64`, `tex_icon_herb_32`
  - `tex_icon_coins_64`, `tex_icon_coins_32`

All 28 icon assets are documented with author credits in `docs/assets/licensing_register.md` and registered in the game's credits subsystem.

### 3.2. External CC0 1.0 Universal Cohort (12 Assets)
- **Sparklin Labs (8 Assets)**: Superpowers Asset Packs (CC0 1.0 Universal) — 8 creature and prop spritesheets/textures (Slime, Snake, Cyclops, Treasure Chest).
- **Kenney (4 Assets)**: Kenney Roguelike/RPG Pack (CC0 1.0 Universal) — 4 environmental biome tileset textures (Desert, Snow, Swamp, Mountain).

### 3.3. In-House Studio Cohort (661 Assets)
Created directly by the Wildbound development and audio teams and dedicated under CC0 1.0 Universal:
- **Wildbound Art Team (526 Assets)**: Characters, cosmetics, modular armor, forest tilesets, environmental props, UI elements, and Corruption Storm visual VFX.
- **Wildbound Audio Team (128 Assets)**: Full sound effect library (combat, creature vocalizations, environment, player movement, UI cues) and original soundtrack tracks.
- **Wildbound Sound Architect (5 Assets)**: Corruption Storm procedural ambience (`amb_storm_rumble`), storm hazard sounds (`sfx_storm_warning`, `sfx_storm_shrink`, `sfx_storm_damage`), and sample chime (`sfx_chime`).
- **Wildbound UI Team (2 Assets)**: Master item icon atlases.

---

## 4. Verification Check

A programmatic scan verified:
1. `commercial_use == True` for all 701 assets.
2. `attribution_required == True` exclusively for the 28 CC BY 3.0 icons.
3. `status == "approved"` for all 701 assets.
4. No GPL, LGPL, CC-NC, CC-ND, or non-commercial licenses exist anywhere in the Wildbound asset manifest.

---

## 5. Conclusion

Wildbound's asset repository is legally pristine, fully licensed for commercial release, and compliant with all intellectual property and attribution standards.


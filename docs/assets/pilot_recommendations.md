# Wildbound — Gate 1 Pilot Asset Recommendations & Feasibility Evaluation

## 1. Pilot Objective

In accordance with Phase 49's strict gate controls, acquisition and integration must begin with a **small, controlled pilot across three targeted categories** before large-scale library ingestion:
1. One Environment Tileset
2. One Character or Creature Spritesheet
3. One UI Icon Pack

This allows the pipeline tools, licensing registers, AMS registration, and rendering passes to be validated end-to-end on a minimal surface area without risking game stability.

---

## 2. Recommended Pilot Candidates

### Pilot 1: Environment Tileset — Kenney Roguelike / Desert Expansion
- **Candidate Identifier**: `pilot_env_desert_kenney`
- **Source**: Kenney (kenney.nl)
- **Source URL**: `https://github.com/KenneyNL/Starter-Kits/tree/master/2D/roguelike-rpg`
- **License**: **CC0 1.0 Universal** (Public Domain dedication)
- **Commercial Usage**: Fully permitted, unrestricted
- **Current Wildbound Gap**: The Desert biome (`assets/tilesets/desert/desert_tileset.png`) contains only a basic 2.5 KB sand swatch. It lacks sandstone cliff walls, desert ruins, steps, and dune elevation transitions.
- **Visual Compatibility**: 2D top-down pixel art with clean palettes.
- **Technical Adaptation**: Sliced from the master atlas into discrete 32×32 tiles; output into `assets/processed/biomes/desert/` with Tiled TSX metadata.
- **Status**: **LICENSE_VERIFIED & RECOMMENDED FOR PILOT**

---

### Pilot 2: Creature Spritesheet — Superpowers Medieval Fantasy Slime
- **Candidate Identifier**: `pilot_creature_slime_superpowers`
- **Source**: Sparklin Labs — Superpowers Asset Packs (sparklinlabs)
- **Source URL**: `https://github.com/sparklinlabs/superpowers-asset-packs/tree/master/medieval-fantasy/monsters/slime`
- **License**: **CC0 1.0 Universal** (Public Domain dedication)
- **Commercial Usage**: Fully permitted, unrestricted
- **Current Wildbound Gap**: Currently only 5 enemy types exist (Goblin, Skeleton, Bat, Boar, Wolf). Wildbound lacks a classic fantasy slime / blob creature for early-tier zones.
- **Visual Compatibility**: 2D top-down 4-directional pixel art animation with clean bounce and attack animations.
- **Technical Adaptation**: Sliced into standard directional walk/idle frames, mapped with `AnimationMetadata` JSON (`anim_slime_idle_down`, `anim_slime_walk_down`, etc.), output into `assets/processed/creatures/slime/`.
- **Status**: **LICENSE_VERIFIED & RECOMMENDED FOR PILOT**

---

### Pilot 3: UI Icon Pack — Game-Icons.net Fantasy Action & Item Icons
- **Candidate Identifier**: `pilot_ui_icons_gameicons`
- **Source**: Game-icons.net (Creators: Lorc & Delapouite)
- **Source URL**: `https://game-icons.net/`
- **License**: **CC BY 3.0** (Attribution required: "Icons by Lorc & Delapouite under CC BY 3.0")
- **Commercial Usage**: Fully permitted with attribution
- **Current Wildbound Gap**: Action bars, corruption storm debuff indicators, and inventory categories require sharp, recognizable fantasy icons.
- **Visual Compatibility**: High-contrast, iconic silhouettes that downsample crisply to 16×16, 24×24, and 32×32 pixel art formats.
- **Technical Adaptation**: Converted to 32×32 RGBA PNGs with dark fantasy borders matching Wildbound's UI theme, registered in `asset_manifest.json` under `ui/icons/`.
- **Status**: **LICENSE_VERIFIED & RECOMMENDED FOR PILOT**

---

## 3. Pilot Verification Criteria (Gate 3 Plan)

Once Gate 2 approval is granted to acquire these pilot assets, the following verification pipeline will execute:
1. **Raw Storage**: Preserved unchanged in `assets/external/kenney/`, `assets/external/superpowers/`, and `assets/external/game_icons/`.
2. **Contact Sheet Generation**: Python generator emits visual contact sheets with grid rulers for inspection.
3. **Processing**: Sliced copies placed in `assets/processed/`.
4. **AMS Loading**: Metadata registered in `assets/manifests/asset_manifest.json`.
5. **Rendering Pass**: Rendered in a test scene and headless CTest validation (`test_assets.exe`).
6. **Regression Testing**: Execute full test suite to guarantee all existing 661 assets function without flaw.


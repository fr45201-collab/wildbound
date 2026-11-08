# Wildbound — External Asset Source Compatibility & Licensing Feasibility

## 1. Overview

This document evaluates the 8 target external free asset libraries for Wildbound's top-down fantasy battle royale aesthetic, technical format compatibility, and legal licensing safety.

---

## 2. Source Evaluation Matrix

| Source | Primary License | Commercial Use Permitted? | Direct URL / Script Download? | Visual Fit with Wildbound | Recommended Category |
|---|---|---|---|---|---|
| **1. Kenney** | **CC0 1.0 Universal** | **YES (Unrestricted)** | **YES** (GitHub / direct zip) | **COMPATIBLE** | UI, Icons, Dungeon/Town Props |
| **2. OpenGameArt** | CC0, CC BY 3.0/4.0 | **YES (Verified per asset)** | **YES** (Direct file links) | **COMPATIBLE / ADAPTABLE** | Creatures, Biome Props, Items |
| **3. itch.io (Free)** | Varies (CC0 / Custom) | **YES (Verified per pack)** | Conditional (Manual or direct) | **COMPATIBLE / ADAPTABLE** | Biome Tiles, Weapons, Monsters |
| **4. CraftPix (Free)** | CraftPix Free License | **YES (In games)** | Web download / Free catalog | **ADAPTABLE** | Monster sprites, Fantasy UI |
| **5. Superpowers** | **CC0 1.0 Universal** | **YES (Unrestricted)** | **YES** (GitHub repo raw links)| **COMPATIBLE** | Medieval tiles, Characters |
| **6. Game-icons.net** | **CC BY 3.0** | **YES (With attribution)** | **YES** (Direct PNG/SVG API) | **COMPATIBLE** | Item Icons, Ability Icons, UI |
| **7. Universal LPC** | CC-BY 3.0 / CC0 | **YES (With attribution)** | **YES** (GitHub generator repo)| **ADAPTABLE (Scale 64->32)**| Modular Armor, Hair, Clothes |
| **8. Lospec** | Public Domain / CC0 | **YES (Unrestricted)** | **YES** (Direct palette files)| **COMPATIBLE** | Color Palettes, Harmonization |

---

## 3. In-Depth Source Feasibility Analysis

### 3.1 Kenney (kenney.nl)
- **Licensing**: All Kenney assets are dedicated to the public domain under **CC0 1.0 Universal**. No royalties, no commercial barriers, attribution appreciated but not legally demanded.
- **Accessibility**: Mirrors are maintained on official GitHub repositories (`KenneyNL/Starter-Kits`, `KenneyNL/Assets`). Direct archive downloads can be automated using standard HTTP requests without authentication.
- **Visual Assessment**: Extremely clean pixel art. UI packs, crosshairs, fantasy icons, and cursor packs integrate with zero modification.

### 3.2 OpenGameArt.org
- **Licensing**: Multiple licenses coexist. We mandate filtering exclusively for **CC0** and **CC BY 3.0 / 4.0**. Any submission bearing CC BY-NC (non-commercial) or unclear authorship is **REJECTED**.
- **Accessibility**: Assets are hosted as direct HTTP download links (`opengameart.org/sites/default/files/...`).
- **Visual Assessment**: Rich repository for 32×32 fantasy tilesets (Dungeon Crawl tiles), classic 4-directional creature walk cycles, and weapon icons.

### 3.3 Superpowers Asset Packs (Sparklin Labs)
- **Licensing**: 100% **CC0 1.0 Universal**. Openly released to the public domain following engine sunset.
- **Accessibility**: Hosted on GitHub (`github.com/sparklinlabs/superpowers-asset-packs`). Every PNG and animation sheet is directly accessible via GitHub raw URLs or repository clones.
- **Visual Assessment**: The `medieval-fantasy` pack contains high-quality top-down characters, skeletons, slimes, goblins, and village architecture perfectly aligned with Wildbound's oblique perspective.

### 3.4 Game-icons.net
- **Licensing**: **CC BY 3.0** (Attribution required: credited to Delapouite, Lorc, etc.). Full commercial rights granted.
- **Accessibility**: Direct downloads for 4,000+ icons in transparent PNG or SVG.
- **Visual Assessment**: Clean, high-contrast monochrome icons. Perfectly suited for UI action bars, inventory items, passive buffs, and corruption storm indicators.

### 3.5 Universal LPC (Liberated Pixel Cup)
- **Licensing**: Base humanoid sprites, clothing layers, and hair elements created by Stephen Challener (Redshrike) are licensed under **CC-BY 3.0** and **GPL 3.0**. Pure CC-BY components are selected.
- **Accessibility**: Hosted in the official LPC repository (`github.com/sanderfrenken/universal-lpc-spritesheet-character-generator`).
- **Visual Assessment**: LPC uses a 64×64 frame grid with 4-directional walk, slash, and spellcast cycles. Integrating with Wildbound's 32×48 player frame requires integer downsampling or adopting LPC's 64×64 proportion for Large humanoid bosses.

### 3.6 CraftPix Free Assets (craftpix.net)
- **Licensing**: CraftPix Free License grants non-exclusive, worldwide permission to use assets in commercial and non-commercial game projects. Prohibits reselling the assets as raw stock.
- **Accessibility**: Free packs can be retrieved directly from official promotional packages.
- **Visual Assessment**: Highly detailed fantasy pixel art. Monsters (orcs, golems, undead) provide excellent boss and elite enemy candidates.

### 3.7 Lospec (lospec.com)
- **Licensing**: Palettes are color value specifications (hexadecimal arrays), legally free of copyright restrictions.
- **Accessibility**: Lospec Palette API provides direct `.hex` and `.png` swatch downloads.
- **Visual Assessment**: Essential for palette re-mapping and unifying disparate external artwork into Wildbound's dark atmospheric fantasy palette.

---

## 4. Incompatible Source Red Flags & Rejection Criteria

To protect the project from legal and technical compromise, the following are strictly prohibited:
1. **CC BY-NC / Personal Use**: Prohibited because Wildbound must retain full commercial rights.
2. **Unverified Web Scrapes / Pinterest / Google Images**: Automatically rejected without provenance.
3. **AI-Generated Hallucinations with Artifacts**: Inconsistent pixel grids, noisy colors, and blurred transparency will not be accepted.
4. **Isometric Pre-Rendered 3D Models**: Incompatible with Wildbound's 2D top-down orthogonal pixel grid.


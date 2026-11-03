# Wildbound Asset Licensing Records

## 1. Licensing Philosophy & Legal Compliance
Wildbound enforces strict licensing records for all game assets.
Every asset entered into the production catalog (`assets/manifests/asset_manifest.json`) must carry verified provenance, author attribution, and explicit commercial use permissions.

## 2. Permitted Licenses
- **Creative Commons Zero (CC0 1.0 Universal)**: Public domain dedication. Unrestricted commercial and non-commercial use with or without attribution.
- **MIT License**: Permissive open-source license. Requires attribution notice.
- **CC-BY (3.0 / 4.0)**: Attribution required. Commercial use permitted with proper crediting in `CREDITS.md`.
- **Custom Commercial Game Studio License**: Explicit written authorization required.

## 3. License Statuses
- **`approved`**: The asset has an identified author, verified source URL, and legal license permitting commercial game redistribution.
- **`license_review_required`**: Provenance or commercial usage rights have not yet been independently verified. Assets with this status cannot enter production release builds and trigger warnings in `AssetValidator`.
- **`missing`**: Listed in metadata but physical file is absent from disk.
- **`deprecated`**: Legacy asset marked for replacement.

## 4. Sample Asset Provenance
| Asset ID | File | Author / Source | License | Status |
|---|---|---|---|---|
| `tex_player` | `characters/player/player_spritesheet.png` | Wildbound Art Team | CC0 1.0 | Approved |
| `sheet_player` | `characters/player/player_spritesheet.json` | Wildbound Art Team | CC0 1.0 | Approved |
| `tex_oak_tree` | `environment/trees/oak_tree.png` | Wildbound Art Team | CC0 1.0 | Approved |
| `tex_item_icons`| `ui/icons/item_icons.png` | Wildbound UI Team | CC0 1.0 | Approved |
| `sfx_chime` | `audio/sfx/sample_chime.wav` | Wildbound Audio Lab | CC0 1.0 | Approved |


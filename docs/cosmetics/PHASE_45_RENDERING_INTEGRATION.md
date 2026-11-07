# WILDBOUND — PHASE 45: RENDERING INTEGRATION SPECIFICATION

## 1. Modular Character Rendering Architecture

Wildbound uses a multi-layered 2D sprite composition system implemented in `wildbound::entities::ModularCharacter`. Characters are rendered by layering individual clothing, armor, accessory, and skin sprite sheets in back-to-front draw order.

```
       [Back Layer (Capes, Cloaks, Backpacks)]
                         v
       [Base Character Body (Skin & Eyes)]
                         v
       [Lower Body Layer (Pants, Trousers, Greaves)]
                         v
       [Footwear Layer (Boots, Shoes, Sabatons)]
                         v
       [Torso Layer (Shirts, Tunics, Armor Chestplates)]
                         v
       [Head Layer (Helmets, Hoods, Bandanas, Hats)]
                         v
       [Weapon & Shield Layer (Right & Left Hands)]
                         v
       [Foreground Accessories (Auras, Belts, Trinkets)]
```

---

## 2. Asset Management System (AMS) Mapping

Every cosmetic in the PostgreSQL database and `costume_catalog.json` defines an `asset_id` (e.g. `sheet_clo_basic_traveler_001`).

### 2.1 Texture Loading & Caching
When `ModularCharacter::EquipSlot(slot, assetId)` is called:
1. `ModularCharacter` checks its internal texture cache.
2. If unmapped, `ResourceManager::GetTexture(assetId)` loads the PNG sprite sheet from `assets/sprites/characters/equipment/`.
3. If the asset cannot be found on disk, the system logs a warning and smoothly renders a fallback texture or renders the base body without crashing.

### 2.2 Animation Frame Alignment
All cosmetic sprite sheets share a uniform grid:
- **Frame Size**: 64 x 64 pixels per frame.
- **Directional Rows**: 4 directions (South = 0, North = 1, East = 2, West = 3).
- **Animation Frames**: 6 walk frames, 4 idle frames, 5 attack frames, 3 hit/death frames.
- Because all sheets adhere to the same coordinate matrix, drawing all layers at the same entity animation index produces pixel-perfect alignment.

---

## 3. Dedicated Server Headless Safety

### 3.1 Strict raylib Decoupling on Server
The dedicated server executable (`WildboundServer.exe`) runs completely headless without GPU or raylib display windows:
- On `WildboundServer`, `ModularCharacter` equipment data holds string identifiers (`skinId`, `chestId`, `legsId`, etc.) and verified asset IDs, but skips calling `LoadTexture` or raylib GL operations.
- The server maintains the authoritative state matrix and broadcasts it via `MsgPlayerAppearanceSync` to graphical clients.
- Graphical game clients receive the message and perform the actual OpenGL draw calls locally.


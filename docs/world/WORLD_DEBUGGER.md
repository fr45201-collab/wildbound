# Wildbound — World Debugger & Developer Tooling Manual

## 1. Overview
Section 7 ("Battle Royale World Debugger") in `DevUI` Tab 4 ("World & Map") provides developers, QA testers, and designers with real-time inspection, diagnostic visual overlays, and instantaneous teleportation controls across the Battle Royale island.

```
+------------------------------------------------------------------------------------+
|               DEVUI — TAB 4: WORLD & MAP — SECTION 7: BR DEBUGGER                  |
|                                                                                    |
| [Status & Metrics]                                                                 |
|   World ID: battle_royale_island        Name: Isle of Wildbound                    |
|   Dimensions: 320x320 (10240x10240)    Playable: (768, 768, 8704, 8704)            |
|   Status: Validated (0 errors)                                                     |
|                                                                                    |
| [Debug Visual Overlays]                                                            |
|   [x] Playable Boundary (Red)           [x] Biome Regions (Multi-color)            |
|   [x] Settlements (Green circles)       [x] Points of Interest (Cyan stars)        |
|   [ ] Spawn Regions (Yellow squares)    [ ] Loot Locations (Gold diamonds)         |
|   [ ] Traversal Routes (Orange lines)   [ ] Chokepoints (Magenta crosses)          |
|                                                                                    |
| [Spatial Teleportation Inspector]                                                  |
|   Select POI / Settlement: [ Greenwood Village             v ] [ Teleport Player ] |
|   Camera Center:           [ (4160, 5920)                  v ] [ Pan Camera      ] |
+------------------------------------------------------------------------------------+
```

---

## 2. Real-Time Visual Overlays (2D In-Game Camera Mode)
When toggled via `DevUI`, `WorldManager::RenderDebugOverlays(camera)` renders world-space diagnostic primitives directly into the game's 2D render loop:

1. **Playable Boundary**:
   * Outlines the safe playable area in semi-transparent red (`RED` with `0.75` alpha).
   * Renders the outer non-playable storm/hazard perimeter.
2. **Biome Regions**:
   * Draws tinted regional boundary rectangles and renders centered biome labels (e.g., *Verdant Forest*, *Sunscorched Wastes*).
3. **Settlements**:
   * Displays bright green boundary circles indicating settlement zones and interaction radii.
4. **Points of Interest (POIs)**:
   * Displays cyan circular markers and text callouts identifying each landmark and its assigned loot tier.
5. **Spawn Regions**:
   * Renders yellow bounding squares around each of the 12 spawn zones and marks individual 48 spawn points.
6. **Loot Locations**:
   * Renders golden diamonds corresponding to container markers, color-coded by tier (Gray for Common, Blue for Rare, Gold for Legendary).
7. **Traversal Routes**:
   * Draws orange paths tracing the 5 primary highways connecting POIs.
8. **Chokepoints**:
   * Displays magenta crossbars and directional vectors across bridges and chasm bottlenecks.

---

## 3. Fast Teleportation & Inspection Controls
Located directly in the ImGui interface of Tab 4:
* **Dropdown Selection**: Populates all 5 settlements and 14 POIs alphabetically or by category.
* **Instant Player Teleport**: Clicking `[Teleport Player]` instantly positions the player character at the designated center coordinates and centers the camera smoothly.
* **Camera Detach / Pan**: Clicking `[Pan Camera]` pans the dev camera to inspect the target zone without moving the player entity.

---

## 4. Integration Source Files
* Definition: `src/world/WorldData.hpp`
* Implementation: `src/world/WorldManager.hpp`, `src/world/WorldManager.cpp`
* DevUI Hooks: `src/ui/DevUI.hpp`, `src/ui/DevUI.cpp` (`DevUI::DrawWorldTab()`)
* Render Pipeline: `src/core/Game.cpp` (in `Game::Render()` within `BeginMode2D(m_camera)`)


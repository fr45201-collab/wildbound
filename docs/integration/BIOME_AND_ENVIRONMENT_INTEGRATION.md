# Biome and Environment Integration

## 1. Overview & Architecture

Wildbound integrates multiple distinct geographical biomes into an expansive top-down world, complemented by interior building navigation and atmospheric environmental animations:

- **Biomes**: Forest (`forest_01`), Desert, Snow, Swamp, Mountain managed by [`BiomeManager`](file:///c:/dev/Wildbound/src/world/BiomeManager.hpp).
- **Village Settlements & Interiors**: Residential cottages, tavern, blacksmith, and storehouse managed by [`BuildingManager`](file:///c:/dev/Wildbound/src/world/BuildingManager.hpp).
- **Environmental Animations**: Animated water ripples, waterfalls, windblown grass, swaying trees, chimneys, and wildlife managed by [`EnvironmentalAnimationSystem`](file:///c:/dev/Wildbound/src/world/environment/EnvironmentalAnimationSystem.hpp).

---

## 2. Integrated Environmental Architecture

```
                       World Simulation (World.cpp)
                                    |
          +-------------------------+-------------------------+
          |                         |                         |
   Biome Configuration      Building Navigation      Environmental Animations
   (BiomeManager.cpp)     (BuildingManager.cpp)       (EnvironmentalAnimation)
   - Active Biome ID       - Entrance triggers        - Water ripples / waterfalls
   - Tileset mappings      - Interior map swapping    - Windblown grass & trees
   - Ambient audio loops   - Seamless camera clamp    - Chimney & campfire smoke
   - Lighting tints        - Audio occlusion          - Flying birds & butterflies
```

---

## 3. Building Interior Transitions

1. **Entrance Prompt**: When player is within $32\text{px}$ of a building entrance, a world prompt is drawn:
   $$\text{"Press [E] or [F] to Enter <Building Name>"}$$
2. **Transition**:
   - `BuildingManager::EnterBuilding(buildingId, world, player, camera, assetMgr)` swaps the active map to the building interior map.
   - Adjusts camera world bounds to interior dimensions.
   - Places player at the designated interior entry waypoint.
   - Muffles exterior ambient wind and shifts audio to interior acoustic reverb.
3. **Exit**:
   - Re-approaching the doorway trigger presents `"Press [E] or [F] to Exit"`.
   - `BuildingManager::ExitBuilding` restores the exterior forest world map and camera boundaries.

---

## 4. Multi-Layer Environmental Animation Rendering

Rendered across three depth passes synchronized with `WorldRenderer`:
1. **Ground Pass (`RenderWorldGround`)**: Water flows, river rapids, pond reflections beneath entities.
2. **Object Pass (`RenderWorldObjects`)**: Swaying oak and pine canopies, rustling berry bushes, flowering tall grass, crackling campfires.
3. **Overhead Pass (`RenderWorldOverhead`)**: Chimney smoke plumes, drifting fog banks, flying birds, roaming butterflies, high-altitude reconnaissance aircraft flyovers.

---

## 5. Verification & Tests

- [`tests/world/test_biomes.cpp`](file:///c:/dev/Wildbound/tests/world/test_biomes.cpp): Multi-biome data loading, active biome swapping, and audio loop transitions.
- [`tests/world/test_village_buildings.cpp`](file:///c:/dev/Wildbound/tests/world/test_village_buildings.cpp): Building entrance detection, interior map loading, and player repositioning.
- [`tests/world/test_environmental_animation.cpp`](file:///c:/dev/Wildbound/tests/world/test_environmental_animation.cpp): Sprite sheet registration, animation state updates, and depth pass rendering.
- [`tests/integration/test_gameplay_integration.cpp`](file:///c:/dev/Wildbound/tests/integration/test_gameplay_integration.cpp): Verified combined map loading, entity spawning, pickup placement, and combat in the living environment.


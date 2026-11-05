# WILDBOUND — ATMOSPHERIC EFFECTS SPECIFICATION

## 1. Overview

The atmospheric effects subsystem creates multi-layered depth and environmental immersion through three distinct phenomena:
1. **Overhead Drifting Clouds**: High-altitude weather bodies casting subtle world ambience.
2. **Dynamic Ground Mist & Fog**: Screen-space atmospheric haze enhancing forest morning and river humidity moods.
3. **Smoke Emitters**: Localized animated chimney, campfire, and ruins plumes that indicate human and magical activity.

---

## 2. Overhead Clouds Subsystem

- **Elevation Layer**: Rendered in `RenderWorldOverhead` above solid structures and character heads ($Z=2$).
- **Movement Model**: Continuous linear horizontal drift driven by simulated wind velocity ($10–25$ px/s).
- **Infinite Wrapping**: When a cloud drifts past the world's right edge $+ 300$ px, its position wraps to the left boundary $- 300$ px with randomized altitude offsets.
- **Blending & Coloration**: Semi-transparent alpha tinting ($0.4–0.7$) allows terrain and tree crowns to remain visible beneath cloud bodies.

---

## 3. Dynamic Fog & Mist Subsystem

- **Canvas-Space Projection**: Rendered during `RenderScreenAtmosphere` inside `m_renderer->BeginUI()` at internal resolution $640 \times 360$.
- **Texture Tiling**: Continuously tiles `tex_fog_light_layer` horizontally and vertically across the active viewport.
- **Scroll Offset**: An internal offset `m_fogScrollOffset` scrolls the fog texture at 12 px/s.
- **Tunable Density**:
  - `m_fogOpacity` controls global fog density from $0.0$ (Clear skies) to $1.0$ (Dense zero-visibility mist).
  - Configurable via developer UI Tab 9 ("Living World").

---

## 4. Smoke Plumes Subsystem

- **Anchor Attachment**: Fixed to world object roots (e.g. cottage chimney caps, campfire hearths, ruined stone pillars).
- **Thermal Dissipation Animation**: 6-frame looping spritesheets featuring volumetric puff growth and alpha dispersal.
- **Phase Offset**: Each instance receives a randomized start timer to prevent synchronized looping across multiple chimneys in a village.


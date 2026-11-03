# Environmental Objects & Prop System — Wildbound Engine

## 1. Overview
Environmental objects are non-tile world entities (such as trees, boulders, buildings, and large bushes) that require:
1. Dynamic Y-depth sorting relative to characters and each other.
2. Independent anchor origins (such as trunk bases or cabin foundations).
3. Precision collision boundaries that only block the grounded portion of the object.

---

## 2. Object Anchor Origins
In Tiled, object coordinate points are anchored at the top-left or bottom-left. Wildbound standardizes entity positions at **bottom-center**:
```
Anchor Origin: Vector2{ width * 0.5f, height }
```
This enables natural depth sorting and accurate collision footings:
* **Render Rectangle**: `[pos.x - origin.x, pos.y - origin.y, width, height]`
* **Collision Rectangle**: `[pos.x - origin.x, pos.y - origin.y + (height - colHeight), colWidth, colHeight]`

---

## 3. Sample Forest Entities

| Entity Type | Texture Key | Dimensions | Collision Shape | Role |
|---|---|---|---|---|
| `oak_tree` | `tex_oak_tree` | 48 × 64 px | 24 × 16 px (trunk base) | Deciduous woodland canopy |
| `pine_tree` | `tex_pine_tree` | 48 × 80 px | 24 × 20 px (trunk base) | Coniferous forest barrier |
| `cabin` | `tex_cabin` | 128 × 128 px | 110 × 55 px (foundation) | Abandoned forest cabin |
| `rock_large` | `tex_rock_large`| 64 × 48 px | 52 × 32 px (boulder base)| Granite river stone |
| `bush_large` | `tex_bush_large`| 48 × 36 px | 36 × 20 px (shrub base) | Berry vegetation |


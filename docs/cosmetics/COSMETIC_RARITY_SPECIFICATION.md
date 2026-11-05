# Cosmetic Rarity Specification (Phase 26)

## 1. Rarity Tiers & Visual Styling

Wildbound defines 6 canonical rarity tiers in `src/cosmetics/CosmeticsRarity.hpp`.
Each tier is assigned distinct UI presentation colors, card border colors, and priority sort weights.

| Tier | Enum Value | Display Name | UI Font Color (RGBA) | Card Border Color (RGBA) | Sort Priority | Hex Code |
|---|---|---|---|---|---|---|
| **Common** | `0` | Common | `(210, 210, 210, 255)` | `(140, 140, 140, 255)` | `0` | `#D2D2D2` |
| **Uncommon** | `1` | Uncommon | `(70, 210, 100, 255)` | `(40, 160, 60, 255)` | `1` | `#46D264` |
| **Rare** | `2` | Rare | `(60, 150, 250, 255)` | `(30, 100, 200, 255)` | `2` | `#3C96FA` |
| **Epic** | `3` | Epic | `(190, 80, 250, 255)` | `(130, 40, 190, 255)` | `3` | `#BE50FA` |
| **Legendary** | `4` | Legendary | `(250, 180, 40, 255)` | `(200, 120, 20, 255)` | `4` | `#FAB428` |
| **Mythic** | `5` | Mythic | `(250, 50, 80, 255)` | `(180, 20, 40, 255)` | `5` | `#FA3250` |

---

## 2. Mathematical Sorting Formulation

When items are sorted by rarity:

$$S_{\text{item}} = P(R_{\text{item}}) \cdot 1000 - \text{lexicographical\_rank}(\text{Name})$$

Where $P(R) \in [0, 5]$ is the rarity sort priority. In descending order ($RarityDesc$):
$$\text{Mythic} > \text{Legendary} > \text{Epic} > \text{Rare} > \text{Uncommon} > \text{Common}$$

---

## 3. Presentation in Store & Wardrobe UI

1. **Card Border**: Renders a 1.5-pixel border colored with `GetRarityMetadata(rarity).borderColor`.
2. **Item Title**: Rendered using `GetRarityMetadata(rarity).uiColor`.
3. **Pill Badge**: Displays uppercase tier name alongside item category (e.g. `LEGENDARY HEADWEAR`).
4. **Featured Filter**: Automatically gathers items with rarity $\ge \text{Epic}$ into the `Featured` tab.


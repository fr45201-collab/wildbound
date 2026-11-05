# Wildbound — Item & Collectible Art Specification

## 1. Visual Direction & Design Principles

The Wildbound item and collectible asset collection adheres strictly to the established Phase 6 Art Bible standards: a cohesive, readable 2D top-down fantasy aesthetic calibrated for a virtual resolution of 640×360 pixels and 32×32 base tile sizing.

### Core Visual Principles
* **Instant Readability at Low Resolution**: Items possess crisp, unambiguous silhouettes with pronounced value contrast so players can immediately distinguish coins, potions, minerals, and artifacts on the ground or within inventory slots.
* **Palette Cohesion**: All item assets utilize the master game color palette (warm gold, antique silver, deep blood rubies, sapphire blues, emerald greens, earthy leathers, timber browns, and ethereal arcane cyan/purple).
* **Dual Presentation Consistency**: Every item is rendered in two purpose-built forms:
  1. A detailed **Inventory Icon** ($64 \times 64$ px) for UI presentation.
  2. A streamlined **World Representation** ($32 \times 32$ px) with contact drop-shadow for in-world ground display.

---

## 2. Technical Asset Standards & Dimensions

| Representation | Dimensions | Canvas / Grid | Framing & Presentation |
| :--- | :--- | :--- | :--- |
| **Inventory Icon** | 64 × 64 px | Single 64×64 cell | Centered subject framed within an elegant circular UI badge (radius 27 px) with dark translucent backing (`#1A1E29CC`), dark steel border, inner specular highlight, and 6–8 px padding. |
| **World Sprite** | 32 × 32 px | Single 32×32 cell | Top-down angled perspective (30° tilt) resting on an elliptical contact drop-shadow (`#00000066`), designed for seamless ground visibility across all 5 biomes. |

---

## 3. Category Visual Styles & Aesthetics

### 1. Collectibles (8 Items)
* **Gold & Silver Coins**: Circular minted tokens with cross/rim stamping, metallic highlights, and rim shading.
* **Faceted Gemstones (Ruby, Sapphire, Emerald)**: Octagonal and diamond cuts with crisp edge reflections, facet contrast, and bright primary color saturation.
* **Ancient Relics & Crystals**: Ethereal cyan/purple glow, chipped stone bodies, golden filigree, and mystical radiance.
* **Golden Emblem**: Circular royal crest with inscribed star emblem and polished brass finish.

### 2. Consumables (10 Items)
* **Alchemical Potions**: Transparent glass phials (round flasks, tall bottles, hexagonal flasks) with cork stoppers, neck cord bindings, liquid surface meniscus, and bubbles.
  - Health: Vivid crimson red.
  - Mana: Arcane sapphire blue.
  - Stamina: Bright emerald green.
  - Antidote: Deep amethyst purple.
* **Herbal & Dietary Provisions**: Fresh green healing sprigs, braided rustic loaf of crusty bread, crisp blushed orchard apple, roasted drumstick on the bone, and leather-wrapped brass canteen.

### 3. Crafting Materials (8 Items)
* **Raw Harvestables**: Hewn oak timber log with bark edge, rough gray granite boulder fragment, raw iron vein ore, and gleaming gold-specked ore rock.
* **Refined Components**: Tanned cowhide pelt, rolled indigo weave fabric bolt, forged rectangular iron bar with beveled edges, and lustrous cast gold ingot.

### 4. Natural Resources (7 Items)
* **Fungi & Flora**: Spotted red cap toadstool, glowing bioluminescent indigo mushroom, 4-petal golden sun-flower, leafy forest curative sprig, cluster of wild ripe blackberries.
* **Biome Specialists**: Spiny desert cactus fruit with prickly pear thorns; frosty crystalline tundra herb with snow-dusted leaves.

### 5. Special Items (7 Items)
* **Keys**: Ornate four-prong ancient celestial temple key; jagged weathered iron dungeon skeleton key.
* **Cartography & Lore**: Tied vellum scroll with wax seal; weathered nautical treasure parchment with island map and red cross.
* **Arcane Artifacts**: Carved obsidian rune tablet with glowing elder script; gold-framed cabochon talisman on braided leather cord; floating geometric artifact with hovering core.

---

## 4. Rarity Visual Conventions & Tiers

Items are assigned one of 5 standard rarity tiers, which are reflected in UI borders, tooltip styling, and DevUI inspection tools:

| Rarity Tier | Border / Accent Color | Visual Characteristics | Example Items |
| :--- | :--- | :--- | :--- |
| **Common** | White / Light Slate (`#CCCCCC`) | Organic, basic raw materials, rustic foods. | `mat_wood_001`, `con_apple_001`, `con_bread_001` |
| **Uncommon** | Vibrant Forest Green (`#44CC44`) | Refined materials, standard potions, wild herbs. | `con_health_potion_small_001`, `mat_iron_ingot_001` |
| **Rare** | Arcane Deep Azure (`#3388FF`) | High-tier potions, cut gems, rare minerals. | `col_ruby_001`, `con_health_potion_large_001`, `mat_gold_ingot_001` |
| **Epic** | Regal Amethyst Purple (`#AA44FF`) | Ancient crystals, enchanted stones, sacred keys. | `col_ancient_crystal_001`, `spc_rune_stone_001`, `spc_ancient_key_001` |
| **Legendary** | Blazing Solar Gold (`#FFAA00`) | Ancient amulets, relics, mysterious artifacts. | `col_ancient_relic_001`, `spc_ancient_amulet_001`, `spc_mysterious_artifact_001` |

---

## 5. Biome Integration & Ground Placement

Every 32×32 world sprite is tested for visual contrast against all biome terrain types:
* **Forest**: Green foliage and dirt pathways.
* **Desert**: High-value yellow and ochre sand dunes.
* **Snow**: Pure white snowfields and blue glacial ice.
* **Swamp**: Dark murky bog waters and deep peat mud.
* **Mountain**: Dark slate stone and gray scree.

The universal contact drop-shadow ($20 \times 6$ px ellipse at 40% opacity) grounds the object on the surface plane, preventing optical floating and ensuring readability regardless of terrain lighting conditions.


# Wildbound — Item Definition and Registry

## 1. Item Categories and Hierarchy
Wildbound categorizes items into authoritative gameplay classifications:

| Category | Enum Value | Default Max Stack | Typical Items |
| :--- | :--- | :--- | :--- |
| **Collectible** | `ItemCategory::Collectible` | 99 | Coins, gems, ancient relics |
| **Consumable** | `ItemCategory::Consumable` | 10 | Potions, rations, healing herbs |
| **Crafting Material** | `ItemCategory::CraftingMaterial` | 99 | Wood, stone, ingots, leather |
| **Natural Resource** | `ItemCategory::NaturalResource` | 99 | Wild berries, mushrooms, herbs |
| **Special Item** | `ItemCategory::SpecialItem` | 1 | Keys, quest maps, ancient amulets |
| **Weapon** | `ItemCategory::Weapon` | 1 | Swords, hunting bows |
| **Armor / Clothing** | `ItemCategory::Armor` / `Clothing` | 1 | Tunics, iron breastplates, boots |
| **Ammunition** | `ItemCategory::Ammunition` | 30 | Iron arrows, bodkin arrows |

---

## 2. Item Definition Schema (`ItemConfig`)
Each item definition includes:
- `id` (`std::string`): Unique string identifier (e.g. `"arrow_iron_001"`, `"con_health_potion_small_001"`).
- `displayName` (`std::string`): Localized or human-readable title.
- `category` (`ItemCategory`): Primary classification.
- `subcategory` (`std::string`): Finer grouping (e.g. `"potion"`, `"arrow"`).
- `description` (`std::string`): Contextual lore and mechanical tooltip.
- `rarityTier` (`std::string`): `"common"`, `"uncommon"`, `"rare"`, `"epic"`, `"legendary"`.
- `stackable` (`bool`): Whether multiple units can occupy a single slot.
- `maxStackSize` (`int`): Maximum unit count allowed per slot.
- `weight` (`float`): Unit weight in kilograms.
- `visuals`: Texture bindings for UI icon (`iconTextureId`) and world representation (`worldTextureId`).
- `consumable`: `ConsumableConfig` containing `healAmount`, `staminaAmount`, and `effectType`.
- `ammunition`: `AmmunitionConfig` containing `compatibleWeaponType` and `bonusDamage`.

---

## 3. Dynamic Ammunition Registration
Ammunition items (`arrow_iron_001`) are registered programmatically via `ItemManager::RegisterDefaultAmmunition()` upon game startup, ensuring that existing asset manifests and catalogues remain modular without redundant duplication.


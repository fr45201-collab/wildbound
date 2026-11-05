#pragma once

#include <raylib.h>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>

namespace wildbound::items {

enum class ItemCategory {
    Collectible = 0,
    Consumable,
    CraftingMaterial,
    NaturalResource,
    SpecialItem,
    Weapon,
    Armor,
    Clothing,
    Ammunition
};

inline std::string_view ItemCategoryToString(ItemCategory cat) {
    switch (cat) {
        case ItemCategory::Collectible:      return "collectible";
        case ItemCategory::Consumable:       return "consumable";
        case ItemCategory::CraftingMaterial: return "material";
        case ItemCategory::NaturalResource:  return "resource";
        case ItemCategory::SpecialItem:      return "special";
        case ItemCategory::Weapon:           return "weapon";
        case ItemCategory::Armor:            return "armor";
        case ItemCategory::Clothing:         return "clothing";
        case ItemCategory::Ammunition:       return "ammunition";
        default:                            return "unknown";
    }
}

inline ItemCategory StringToItemCategory(std::string_view str) {
    if (str == "collectible") return ItemCategory::Collectible;
    if (str == "consumable")  return ItemCategory::Consumable;
    if (str == "material" || str == "crafting") return ItemCategory::CraftingMaterial;
    if (str == "resource" || str == "natural")  return ItemCategory::NaturalResource;
    if (str == "special")     return ItemCategory::SpecialItem;
    if (str == "weapon" || str == "sword" || str == "bow") return ItemCategory::Weapon;
    if (str == "armor")       return ItemCategory::Armor;
    if (str == "clothing" || str == "costume") return ItemCategory::Clothing;
    if (str == "ammunition" || str == "ammo" || str == "arrow") return ItemCategory::Ammunition;
    return ItemCategory::Collectible;
}

struct ItemVisuals {
    std::string iconTextureId;
    std::string worldTextureId;
};

struct ConsumableConfig {
    float healAmount = 0.0f;
    float staminaAmount = 0.0f;
    std::string effectType = "heal"; // "heal", "stamina", "antidote", "none"
};

struct AmmunitionConfig {
    std::string compatibleWeaponType = "bow";
    float projectileSpeed = 480.0f;
    float bonusDamage = 0.0f;
};

struct WeaponItemConfig {
    std::string weaponId;
    float baseDamage = 20.0f;
    float attackRange = 32.0f;
    float attackSpeed = 1.0f;
    std::string weaponType = "sword"; // "sword", "bow"
};

struct ArmorItemConfig {
    std::string cosmeticId;
    std::string slot = "chest"; // "head", "chest", "legs", "feet", "face", "back"
    float defenseRating = 10.0f;
    float damageReduction = 0.10f;
};

struct ItemConfig {
    std::string id;
    std::string displayName;
    ItemCategory category = ItemCategory::Collectible;
    std::string subcategory;
    std::string description;
    ItemVisuals visuals;
    std::vector<std::string> biomeTags;
    std::vector<std::string> gameplayTags;
    std::string rarityTier = "common";
    Vector2 worldDimensions{ 32.0f, 32.0f };
    bool stackable = true;
    int maxStackSize = 99;
    float weight = 0.1f;

    // Subsystem specific configurations
    ConsumableConfig consumable;
    WeaponItemConfig weapon;
    ArmorItemConfig armor;
    AmmunitionConfig ammunition;

    [[nodiscard]] bool IsValid() const {
        return !id.empty() && !displayName.empty() &&
               !visuals.iconTextureId.empty() && !visuals.worldTextureId.empty();
    }

    [[nodiscard]] bool HasBiomeTag(const std::string& tag) const {
        return std::find(biomeTags.begin(), biomeTags.end(), tag) != biomeTags.end();
    }

    [[nodiscard]] bool HasGameplayTag(const std::string& tag) const {
        return std::find(gameplayTags.begin(), gameplayTags.end(), tag) != gameplayTags.end();
    }
};

using ItemDefinition = ItemConfig;

} // namespace wildbound::items


#pragma once

#include <raylib.h>
#include <string>
#include <string_view>
#include <vector>
#include "../graphics/animation/Direction.hpp"

namespace wildbound::weapons {

enum class WeaponCategory {
    Sword = 0,
    Bow,
    Ammunition
};

inline std::string_view WeaponCategoryToString(WeaponCategory cat) {
    switch (cat) {
        case WeaponCategory::Sword:      return "sword";
        case WeaponCategory::Bow:        return "bow";
        case WeaponCategory::Ammunition: return "ammunition";
        default:                         return "unknown";
    }
}

inline WeaponCategory StringToWeaponCategory(std::string_view str) {
    if (str == "sword")      return WeaponCategory::Sword;
    if (str == "bow")        return WeaponCategory::Bow;
    if (str == "ammunition") return WeaponCategory::Ammunition;
    return WeaponCategory::Sword;
}

/**
 * 6-Tier Weapon Rarity Classification.
 * Each rarity modifies baseline weapon damage, critical effectiveness, and UI representation.
 */
enum class WeaponRarity {
    Common = 0,
    Uncommon,
    Rare,
    Epic,
    Legendary,
    Mythic
};

inline std::string_view WeaponRarityToString(WeaponRarity rarity) {
    switch (rarity) {
        case WeaponRarity::Common:    return "Common";
        case WeaponRarity::Uncommon:  return "Uncommon";
        case WeaponRarity::Rare:      return "Rare";
        case WeaponRarity::Epic:      return "Epic";
        case WeaponRarity::Legendary: return "Legendary";
        case WeaponRarity::Mythic:    return "Mythic";
        default:                      return "Common";
    }
}

inline WeaponRarity StringToWeaponRarity(std::string_view str) {
    if (str == "Common" || str == "common")       return WeaponRarity::Common;
    if (str == "Uncommon" || str == "uncommon")   return WeaponRarity::Uncommon;
    if (str == "Rare" || str == "rare")           return WeaponRarity::Rare;
    if (str == "Epic" || str == "epic")           return WeaponRarity::Epic;
    if (str == "Legendary" || str == "legendary") return WeaponRarity::Legendary;
    if (str == "Mythic" || str == "mythic")       return WeaponRarity::Mythic;
    return WeaponRarity::Common;
}

inline Color GetRarityColor(WeaponRarity rarity) {
    switch (rarity) {
        case WeaponRarity::Common:    return Color{ 190, 195, 205, 255 }; // Light slate gray
        case WeaponRarity::Uncommon:  return Color{ 75, 205, 110, 255 };  // Forest green
        case WeaponRarity::Rare:      return Color{ 65, 155, 245, 255 };  // Sky azure
        case WeaponRarity::Epic:      return Color{ 185, 90, 235, 255 };  // Royal violet
        case WeaponRarity::Legendary: return Color{ 250, 180, 45, 255 };  // Sunlit gold
        case WeaponRarity::Mythic:    return Color{ 240, 55, 80, 255 };   // Crimson flame
        default:                      return Color{ 255, 255, 255, 255 };
    }
}

inline float GetRarityDamageMultiplier(WeaponRarity rarity) {
    switch (rarity) {
        case WeaponRarity::Common:    return 1.00f;
        case WeaponRarity::Uncommon:  return 1.05f;
        case WeaponRarity::Rare:      return 1.10f;
        case WeaponRarity::Epic:      return 1.15f;
        case WeaponRarity::Legendary: return 1.20f;
        case WeaponRarity::Mythic:    return 1.25f;
        default:                      return 1.00f;
    }
}

/**
 * Combat Attack Lifecycle States.
 */
enum class WeaponAttackState {
    Idle = 0,
    Preparation,
    Active,
    Recovery,
    Cooldown
};

inline std::string_view WeaponAttackStateToString(WeaponAttackState state) {
    switch (state) {
        case WeaponAttackState::Idle:        return "Idle";
        case WeaponAttackState::Preparation: return "Preparation";
        case WeaponAttackState::Active:      return "Active";
        case WeaponAttackState::Recovery:    return "Recovery";
        case WeaponAttackState::Cooldown:    return "Cooldown";
        default:                             return "Idle";
    }
}

/**
 * Data-Driven Combat & Weapon Statistics.
 */
struct WeaponStats {
    float baseDamage = 20.0f;
    float attackSpeed = 1.0f;       // Attacks per second
    float attackCooldown = 0.5f;    // Seconds between attack attempts
    float range = 32.0f;            // Reach / Hitbox forward distance
    float hitboxWidth = 32.0f;      // Melee hitbox width
    float hitboxHeight = 24.0f;     // Melee hitbox height
    float critChance = 0.05f;       // 5% default critical hit rate
    float critMultiplier = 1.5f;    // 1.5x critical damage multiplier
    float staminaCost = 10.0f;      // Stamina consumed per swing/shot
    float meleeArc = 45.0f;         // Swing arc in degrees
    float drawDuration = 0.45f;     // Seconds required for full bow draw
    float projectileSpeed = 480.0f; // Velocity of spawned arrow in world units/sec
};

struct WeaponVisuals {
    std::string iconTextureId;
    std::string groundTextureId;
    std::string heldTextureId;
    std::string heldSheetId;
    std::string animTextureId;
    std::string animSheetId;
};

struct WeaponAttachment {
    Vector2 gripAnchor{ 12.0f, 18.0f };
    std::string hand = "right_hand";
    int renderOrderDown = 85;
    int renderOrderUp = 18;
    int renderOrderLeft = 85;
    int renderOrderRight = 85;

    [[nodiscard]] int GetRenderOrder(graphics::animation::Direction dir) const {
        switch (dir) {
            case graphics::animation::Direction::Down:  return renderOrderDown;
            case graphics::animation::Direction::Up:    return renderOrderUp;
            case graphics::animation::Direction::Left:  return renderOrderLeft;
            case graphics::animation::Direction::Right: return renderOrderRight;
            default:                                   return renderOrderDown;
        }
    }
};

struct WeaponConfig {
    std::string id;
    std::string displayName;
    WeaponCategory category = WeaponCategory::Sword;
    std::string subcategory;
    std::string description;
    WeaponVisuals visuals;
    WeaponAttachment attachment;
    std::vector<std::string> directions = { "down", "up", "left", "right" };
    std::vector<std::string> animationFrames = { "idle", "prep", "swing", "recovery" };
    WeaponRarity defaultRarity = WeaponRarity::Common;
    WeaponStats stats;

    [[nodiscard]] bool IsValid() const {
        return !id.empty() && !displayName.empty() &&
               !visuals.iconTextureId.empty() && !visuals.groundTextureId.empty();
    }
};

} // namespace wildbound::weapons

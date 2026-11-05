#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include "../entities/ModularCharacter.hpp"
#include "../graphics/animation/Direction.hpp"

namespace wildbound::customization {

/**
 * High-level cosmetic customization slots (Phase 25).
 * Maps cleanly to existing entities::CosmeticCategory and provides explicit slots
 * for Hairstyle, Headwear, Helmet, Face, Chest, Legs, Feet, Back, and Skin.
 */
enum class CustomizationSlot : uint8_t {
    Skin = 0,
    Hair,
    Headwear,
    Helmet,
    Face,
    Chest,
    Legs,
    Feet,
    Back,
    Count
};

inline std::string_view CustomizationSlotToString(CustomizationSlot slot) {
    switch (slot) {
        case CustomizationSlot::Skin:     return "Skin";
        case CustomizationSlot::Hair:     return "Hair";
        case CustomizationSlot::Headwear: return "Headwear";
        case CustomizationSlot::Helmet:   return "Helmet";
        case CustomizationSlot::Face:     return "Face";
        case CustomizationSlot::Chest:    return "Chest";
        case CustomizationSlot::Legs:     return "Legs";
        case CustomizationSlot::Feet:     return "Feet";
        case CustomizationSlot::Back:     return "Back";
        default:                          return "Unknown";
    }
}

inline CustomizationSlot StringToCustomizationSlot(std::string_view str) {
    if (str == "skin" || str == "Skin" || str == "variation" || str == "body") {
        return CustomizationSlot::Skin;
    }
    if (str == "hair" || str == "Hair" || str == "hairstyle") {
        return CustomizationSlot::Hair;
    }
    if (str == "headwear" || str == "Headwear" || str == "hat" || str == "hood" || str == "cap") {
        return CustomizationSlot::Headwear;
    }
    if (str == "helmet" || str == "Helmet" || str == "helm") {
        return CustomizationSlot::Helmet;
    }
    if (str == "face" || str == "Face" || str == "mask" || str == "goggles" || str == "glasses") {
        return CustomizationSlot::Face;
    }
    if (str == "chest" || str == "Chest" || str == "shirt" || str == "clothing" || str == "armor" || str == "tunic" || str == "robe") {
        return CustomizationSlot::Chest;
    }
    if (str == "legs" || str == "Legs" || str == "pants" || str == "trousers") {
        return CustomizationSlot::Legs;
    }
    if (str == "feet" || str == "Feet" || str == "shoes" || str == "boots" || str == "footwear") {
        return CustomizationSlot::Feet;
    }
    if (str == "back" || str == "Back" || str == "backpack" || str == "quiver" || str == "pack") {
        return CustomizationSlot::Back;
    }
    return CustomizationSlot::Chest;
}

/**
 * Maps a CustomizationSlot to the underlying entities::CosmeticCategory used by ModularCharacter.
 */
inline entities::CosmeticCategory CustomizationSlotToCategory(CustomizationSlot slot) {
    switch (slot) {
        case CustomizationSlot::Skin:     return entities::CosmeticCategory::Skin;
        case CustomizationSlot::Hair:     return entities::CosmeticCategory::Hair;
        case CustomizationSlot::Headwear: return entities::CosmeticCategory::Headwear;
        case CustomizationSlot::Helmet:   return entities::CosmeticCategory::Headwear;
        case CustomizationSlot::Face:     return entities::CosmeticCategory::Face;
        case CustomizationSlot::Chest:    return entities::CosmeticCategory::Shirt;
        case CustomizationSlot::Legs:     return entities::CosmeticCategory::Pants;
        case CustomizationSlot::Feet:     return entities::CosmeticCategory::Shoes;
        case CustomizationSlot::Back:     return entities::CosmeticCategory::Back;
        default:                          return entities::CosmeticCategory::Shirt;
    }
}

/**
 * Converts entities::CosmeticCategory to CustomizationSlot.
 */
inline CustomizationSlot CategoryToCustomizationSlot(entities::CosmeticCategory cat) {
    switch (cat) {
        case entities::CosmeticCategory::Skin:     return CustomizationSlot::Skin;
        case entities::CosmeticCategory::Hair:     return CustomizationSlot::Hair;
        case entities::CosmeticCategory::Headwear: return CustomizationSlot::Headwear;
        case entities::CosmeticCategory::Face:
        case entities::CosmeticCategory::Goggles:
        case entities::CosmeticCategory::Mask:     return CustomizationSlot::Face;
        case entities::CosmeticCategory::Shirt:
        case entities::CosmeticCategory::Armor:    return CustomizationSlot::Chest;
        case entities::CosmeticCategory::Pants:    return CustomizationSlot::Legs;
        case entities::CosmeticCategory::Shoes:    return CustomizationSlot::Feet;
        case entities::CosmeticCategory::Back:     return CustomizationSlot::Back;
        default:                                   return CustomizationSlot::Chest;
    }
}

} // namespace wildbound::customization


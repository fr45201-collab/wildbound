#pragma once

#include "CosmeticTypes.hpp"
#include <string>
#include <vector>
#include <raylib.h>
#include <nlohmann/json.hpp>
#include "../graphics/animation/Direction.hpp"

namespace wildbound::customization {

struct CosmeticDefinition {
    std::string id;
    std::string displayName;
    std::string description;
    CustomizationSlot slot = CustomizationSlot::Chest;
    std::string category = "clothing";
    std::string spritesheetId;
    std::string textureId;
    std::string iconId;
    std::vector<std::string> compatibleDirections = { "up", "down", "left", "right" };
    std::vector<std::string> compatibleAnimations = { "idle", "walk", "run" };
    std::vector<std::string> occlusionFlags; // e.g. "hair_hide_all", "hair_hide_top", "mask_hide_goggles"
    Vector2 anchorOffset{ 0.0f, 0.0f };
    Color tint = WHITE;
    std::string rarity = "common"; // common, uncommon, rare, epic, legendary
    bool isGameplayEquipment = false;
    std::string equipmentItemDefId; // Corresponding item definition in ItemManager if gameplay equipment

    [[nodiscard]] bool IsValid() const noexcept {
        return !id.empty() && !spritesheetId.empty();
    }

    [[nodiscard]] bool SupportsDirection(graphics::animation::Direction dir) const {
        if (compatibleDirections.empty()) return true;
        std::string dirStr = std::string(graphics::animation::DirectionToString(dir));
        for (const auto& d : compatibleDirections) {
            if (d == dirStr) return true;
        }
        return false;
    }

    [[nodiscard]] bool SupportsAnimation(const std::string& animName) const {
        if (compatibleAnimations.empty()) return true;
        for (const auto& a : compatibleAnimations) {
            if (animName.find(a) != std::string::npos) return true;
        }
        return false;
    }

    [[nodiscard]] entities::CosmeticItem ToCosmeticItem() const {
        entities::CosmeticItem item;
        item.id = id;
        item.name = displayName;
        item.category = CustomizationSlotToCategory(slot);
        item.spritesheetId = spritesheetId;
        item.textureId = textureId;
        item.occlusionFlags = occlusionFlags;
        item.anchorOffset = anchorOffset;
        item.tint = tint;
        item.visible = true;
        return item;
    }

    nlohmann::json ToJson() const;
    static CosmeticDefinition FromJson(const nlohmann::json& j);
};

} // namespace wildbound::customization


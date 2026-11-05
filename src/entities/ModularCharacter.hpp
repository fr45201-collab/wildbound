#pragma once

#include <raylib.h>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>
#include "../graphics/animation/Direction.hpp"

namespace wildbound::graphics {
    class Renderer;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::graphics::animation {
    class AnimationController;
}

namespace wildbound::entities {

enum class CosmeticCategory {
    Skin,
    Hair,
    Shirt,
    Armor,
    Pants,
    Shoes,
    Headwear,
    Face,
    Goggles,
    Mask,
    Back,
    Waist
};

inline std::string_view CosmeticCategoryToString(CosmeticCategory cat) {
    switch (cat) {
        case CosmeticCategory::Skin:     return "skin";
        case CosmeticCategory::Hair:     return "hair";
        case CosmeticCategory::Shirt:    return "shirt";
        case CosmeticCategory::Armor:    return "armor";
        case CosmeticCategory::Pants:    return "pants";
        case CosmeticCategory::Shoes:    return "shoes";
        case CosmeticCategory::Headwear: return "headwear";
        case CosmeticCategory::Face:     return "face";
        case CosmeticCategory::Goggles:  return "goggles";
        case CosmeticCategory::Mask:     return "mask";
        case CosmeticCategory::Back:     return "back";
        case CosmeticCategory::Waist:    return "waist";
        default:                         return "unknown";
    }
}

inline CosmeticCategory StringToCosmeticCategory(std::string_view str) {
    if (str == "skin")     return CosmeticCategory::Skin;
    if (str == "hair")     return CosmeticCategory::Hair;
    if (str == "shirt" || str == "clothing") return CosmeticCategory::Shirt;
    if (str == "armor")    return CosmeticCategory::Armor;
    if (str == "pants")    return CosmeticCategory::Pants;
    if (str == "shoes" || str == "footwear") return CosmeticCategory::Shoes;
    if (str == "headwear" || str == "hat") return CosmeticCategory::Headwear;
    if (str == "face")     return CosmeticCategory::Face;
    if (str == "goggles")  return CosmeticCategory::Goggles;
    if (str == "mask")     return CosmeticCategory::Mask;
    if (str == "back" || str == "backpack" || str == "backpacks") return CosmeticCategory::Back;
    if (str == "waist")    return CosmeticCategory::Waist;
    return CosmeticCategory::Skin;
}

struct CosmeticItem {
    std::string id;
    std::string name;
    CosmeticCategory category = CosmeticCategory::Shirt;
    std::string spritesheetId;
    std::string textureId;
    std::vector<std::string> occlusionFlags; // e.g. "hair_hide_all", "hair_hide_top"
    Color tint = WHITE;
    Vector2 anchorOffset{ 0.0f, 0.0f };
    bool visible = true;

    bool IsValid() const { return !id.empty() && (!spritesheetId.empty() || !textureId.empty()); }

    nlohmann::json ToJson() const;
    static CosmeticItem FromJson(const nlohmann::json& j);
};

struct CompatibilityResult {
    bool compatible = true;
    std::string reason;
};

class CharacterEquipment {
public:
    CharacterEquipment() = default;

    CompatibilityResult CheckCompatibility(const CosmeticItem& item) const;
    bool Equip(const CosmeticItem& item, bool force = true);
    bool Unequip(CosmeticCategory category);
    void Clear();

    const CosmeticItem* GetEquipped(CosmeticCategory category) const;
    bool HasEquipped(CosmeticCategory category) const;
    const std::unordered_map<CosmeticCategory, CosmeticItem>& GetAllEquipped() const { return m_equipped; }

    bool HasOcclusionFlag(const std::string& flag) const;

    void ResetToDefault();

private:
    std::unordered_map<CosmeticCategory, CosmeticItem> m_equipped;
};

struct VisualLayer {
    std::string name;
    CosmeticCategory category = CosmeticCategory::Skin;
    std::string spritesheetId;
    std::string textureId;
    Color tint = WHITE;
    Vector2 anchorOffset{ 0.0f, 0.0f };
    int zIndex = 0;
    bool visible = true;
};

class ModularCharacter {
public:
    ModularCharacter();

    void SetBaseBody(const std::string& spritesheetId);
    const std::string& GetBaseBody() const { return m_baseBodySpritesheet; }

    CharacterEquipment& GetEquipment() { return m_equipment; }
    const CharacterEquipment& GetEquipment() const { return m_equipment; }

    // Generates directional sorted layer stack (resolves front/back occlusion)
    std::vector<VisualLayer> BuildLayerStack(graphics::animation::Direction facing) const;

    // Renders all equipped visual layers in lockstep with the character animation
    void Render(graphics::Renderer& renderer,
                resources::AssetManager& assetMgr,
                Vector2 position,
                graphics::animation::Direction facing,
                const graphics::animation::AnimationController& animCtrl);

    // Appearance profile serialization
    nlohmann::json ToJson() const;
    bool FromJson(const nlohmann::json& j, const resources::AssetManager& assetMgr);

    bool LoadAppearance(const std::string& jsonPath, const resources::AssetManager& assetMgr);
    bool SaveAppearance(const std::string& jsonPath) const;

    // Default appearance preset
    void ApplyDefaultAppearance();

private:
    std::string m_baseBodySpritesheet = "player_spritesheet";
    CharacterEquipment m_equipment;
    bool m_renderShadow = true;
};

} // namespace wildbound::entities


#include "ModularCharacter.hpp"
#include "../graphics/Renderer.hpp"
#include "../resources/AssetManager.hpp"
#include "../resources/SpriteSheet.hpp"
#include "../graphics/animation/AnimationController.hpp"
#include "../core/Logger.hpp"
#include <algorithm>
#include <fstream>

using wildbound::core::Logger;

namespace wildbound::entities {

// ============================================================================
// CosmeticItem JSON Serialization
// ============================================================================

nlohmann::json CosmeticItem::ToJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["name"] = name;
    j["category"] = std::string(CosmeticCategoryToString(category));
    j["spritesheet"] = spritesheetId;
    j["texture"] = textureId;
    j["occlusion_flags"] = occlusionFlags;
    j["tint"] = { tint.r, tint.g, tint.b, tint.a };
    j["anchor_offset"] = { anchorOffset.x, anchorOffset.y };
    j["visible"] = visible;
    return j;
}

CosmeticItem CosmeticItem::FromJson(const nlohmann::json& j) {
    CosmeticItem item;
    item.id = j.value("id", "");
    item.name = j.value("name", "Unnamed Cosmetic");
    item.category = StringToCosmeticCategory(j.value("category", "shirt"));
    item.spritesheetId = j.value("spritesheet", "");
    item.textureId = j.value("texture", "");

    if (j.contains("occlusion_flags") && j["occlusion_flags"].is_array()) {
        item.occlusionFlags = j["occlusion_flags"].get<std::vector<std::string>>();
    }

    if (j.contains("tint") && j["tint"].is_array() && j["tint"].size() >= 4) {
        item.tint = Color{
            static_cast<unsigned char>(j["tint"][0].get<int>()),
            static_cast<unsigned char>(j["tint"][1].get<int>()),
            static_cast<unsigned char>(j["tint"][2].get<int>()),
            static_cast<unsigned char>(j["tint"][3].get<int>())
        };
    } else {
        item.tint = WHITE;
    }

    if (j.contains("anchor_offset") && j["anchor_offset"].is_array() && j["anchor_offset"].size() >= 2) {
        item.anchorOffset.x = j["anchor_offset"][0].get<float>();
        item.anchorOffset.y = j["anchor_offset"][1].get<float>();
    }

    item.visible = j.value("visible", true);
    return item;
}

// ============================================================================
// CharacterEquipment Implementation
// ============================================================================

CompatibilityResult CharacterEquipment::CheckCompatibility(const CosmeticItem& item) const {
    CompatibilityResult res;
    // Check 1: Goggles vs Full-Face Mask conflict
    if (item.category == CosmeticCategory::Goggles || item.category == CosmeticCategory::Face) {
        if (HasOcclusionFlag("mask_hide_goggles")) {
            res.compatible = false;
            res.reason = "Equipped full-face mask prevents wearing eyewear/goggles.";
            return res;
        }
    }

    if (item.category == CosmeticCategory::Mask) {
        bool hidesGoggles = std::find(item.occlusionFlags.begin(), item.occlusionFlags.end(), "mask_hide_goggles") != item.occlusionFlags.end();
        if (hidesGoggles && (HasEquipped(CosmeticCategory::Goggles) || HasEquipped(CosmeticCategory::Face))) {
            res.compatible = false;
            res.reason = "Full-face mask conflicts with currently equipped eyewear/goggles.";
            return res;
        }
    }

    return res;
}

bool CharacterEquipment::Equip(const CosmeticItem& item, bool force) {
    if (!item.IsValid()) {
        Logger::Warn("EQUIPMENT", "Attempted to equip invalid item: " + item.id);
        return false;
    }

    if (!force) {
        auto compat = CheckCompatibility(item);
        if (!compat.compatible) {
            Logger::Warn("EQUIPMENT", "Incompatible equipment: " + compat.reason);
            return false;
        }
    }

    // Replace item in same category (category exclusivity)
    m_equipped[item.category] = item;
    Logger::Debug("EQUIPMENT", "Equipped [" + item.id + "] in category " +
                  std::string(CosmeticCategoryToString(item.category)));
    return true;
}

bool CharacterEquipment::Unequip(CosmeticCategory category) {
    auto it = m_equipped.find(category);
    if (it != m_equipped.end()) {
        std::string unequippedId = it->second.id;
        m_equipped.erase(it);
        Logger::Debug("EQUIPMENT", "Unequipped item [" + unequippedId + "] from category " +
                      std::string(CosmeticCategoryToString(category)));
        return true;
    }
    return false;
}

void CharacterEquipment::Clear() {
    m_equipped.clear();
}

const CosmeticItem* CharacterEquipment::GetEquipped(CosmeticCategory category) const {
    auto it = m_equipped.find(category);
    if (it != m_equipped.end()) {
        return &it->second;
    }
    return nullptr;
}

bool CharacterEquipment::HasEquipped(CosmeticCategory category) const {
    return m_equipped.find(category) != m_equipped.end();
}

bool CharacterEquipment::HasOcclusionFlag(const std::string& flag) const {
    for (const auto& [cat, item] : m_equipped) {
        for (const auto& f : item.occlusionFlags) {
            if (f == flag) return true;
        }
    }
    return false;
}

void CharacterEquipment::ResetToDefault() {
    Clear();
}

// ============================================================================
// ModularCharacter Implementation
// ============================================================================

ModularCharacter::ModularCharacter() {
    m_baseBodySpritesheet = "player_spritesheet";
}

void ModularCharacter::SetBaseBody(const std::string& spritesheetId) {
    m_baseBodySpritesheet = spritesheetId;
}

std::vector<VisualLayer> ModularCharacter::BuildLayerStack(graphics::animation::Direction facing) const {
    std::vector<VisualLayer> stack;
    stack.reserve(12);

    bool hideHair = m_equipment.HasOcclusionFlag("hair_hide_all");
    bool hidePants = m_equipment.HasOcclusionFlag("robe_hide_pants");

    // 1. Skin / Base Body Layer
    VisualLayer bodyLayer;
    bodyLayer.name = "body";
    bodyLayer.category = CosmeticCategory::Skin;
    bodyLayer.spritesheetId = m_baseBodySpritesheet;
    const CosmeticItem* skinItem = m_equipment.GetEquipped(CosmeticCategory::Skin);
    if (skinItem && skinItem->IsValid()) {
        bodyLayer.spritesheetId = skinItem->spritesheetId;
        bodyLayer.textureId = skinItem->textureId;
        bodyLayer.tint = skinItem->tint;
    }
    bodyLayer.zIndex = 20;
    stack.push_back(bodyLayer);

    // 2. Back Accessory (Facing-dependent layer order)
    const CosmeticItem* backItem = m_equipment.GetEquipped(CosmeticCategory::Back);
    if (backItem && backItem->IsValid() && backItem->visible) {
        VisualLayer backLayer;
        backLayer.name = "back";
        backLayer.category = CosmeticCategory::Back;
        backLayer.spritesheetId = backItem->spritesheetId;
        backLayer.textureId = backItem->textureId;
        backLayer.tint = backItem->tint;
        backLayer.anchorOffset = backItem->anchorOffset;
        // Facing Up (North): Backpack is IN FRONT of shirt/body (Z = 85)
        // Facing Down (South) / Side: Backpack is BEHIND body (Z = 10)
        backLayer.zIndex = (facing == graphics::animation::Direction::Up) ? 85 : 10;
        stack.push_back(backLayer);
    }

    // 3. Pants / Legwear
    if (!hidePants) {
        const CosmeticItem* pantsItem = m_equipment.GetEquipped(CosmeticCategory::Pants);
        if (pantsItem && pantsItem->IsValid() && pantsItem->visible) {
            VisualLayer pantsLayer;
            pantsLayer.name = "pants";
            pantsLayer.category = CosmeticCategory::Pants;
            pantsLayer.spritesheetId = pantsItem->spritesheetId;
            pantsLayer.textureId = pantsItem->textureId;
            pantsLayer.tint = pantsItem->tint;
            pantsLayer.anchorOffset = pantsItem->anchorOffset;
            pantsLayer.zIndex = 30;
            stack.push_back(pantsLayer);
        }
    }

    // 4. Shoes / Boots
    const CosmeticItem* shoesItem = m_equipment.GetEquipped(CosmeticCategory::Shoes);
    if (shoesItem && shoesItem->IsValid() && shoesItem->visible) {
        VisualLayer shoesLayer;
        shoesLayer.name = "shoes";
        shoesLayer.category = CosmeticCategory::Shoes;
        shoesLayer.spritesheetId = shoesItem->spritesheetId;
        shoesLayer.textureId = shoesItem->textureId;
        shoesLayer.tint = shoesItem->tint;
        shoesLayer.anchorOffset = shoesItem->anchorOffset;
        shoesLayer.zIndex = 40;
        stack.push_back(shoesLayer);
    }

    // 5. Shirt / Clothing
    const CosmeticItem* shirtItem = m_equipment.GetEquipped(CosmeticCategory::Shirt);
    if (shirtItem && shirtItem->IsValid() && shirtItem->visible) {
        VisualLayer shirtLayer;
        shirtLayer.name = "shirt";
        shirtLayer.category = CosmeticCategory::Shirt;
        shirtLayer.spritesheetId = shirtItem->spritesheetId;
        shirtLayer.textureId = shirtItem->textureId;
        shirtLayer.tint = shirtItem->tint;
        shirtLayer.anchorOffset = shirtItem->anchorOffset;
        shirtLayer.zIndex = 50;
        stack.push_back(shirtLayer);
    }

    // 5b. Armor / Cuirass (Layered on top of shirt)
    const CosmeticItem* armorItem = m_equipment.GetEquipped(CosmeticCategory::Armor);
    if (armorItem && armorItem->IsValid() && armorItem->visible) {
        VisualLayer armorLayer;
        armorLayer.name = "armor";
        armorLayer.category = CosmeticCategory::Armor;
        armorLayer.spritesheetId = armorItem->spritesheetId;
        armorLayer.textureId = armorItem->textureId;
        armorLayer.tint = armorItem->tint;
        armorLayer.anchorOffset = armorItem->anchorOffset;
        armorLayer.zIndex = 52;
        stack.push_back(armorLayer);
    }

    // 6. Waist / Belts
    const CosmeticItem* waistItem = m_equipment.GetEquipped(CosmeticCategory::Waist);
    if (waistItem && waistItem->IsValid() && waistItem->visible) {
        VisualLayer waistLayer;
        waistLayer.name = "waist";
        waistLayer.category = CosmeticCategory::Waist;
        waistLayer.spritesheetId = waistItem->spritesheetId;
        waistLayer.textureId = waistItem->textureId;
        waistLayer.tint = waistItem->tint;
        waistLayer.anchorOffset = waistItem->anchorOffset;
        waistLayer.zIndex = 55;
        stack.push_back(waistLayer);
    }

    // 7. Hair
    if (!hideHair) {
        const CosmeticItem* hairItem = m_equipment.GetEquipped(CosmeticCategory::Hair);
        if (hairItem && hairItem->IsValid() && hairItem->visible) {
            VisualLayer hairLayer;
            hairLayer.name = "hair";
            hairLayer.category = CosmeticCategory::Hair;
            hairLayer.spritesheetId = hairItem->spritesheetId;
            hairLayer.textureId = hairItem->textureId;
            hairLayer.tint = hairItem->tint;
            hairLayer.anchorOffset = hairItem->anchorOffset;
            hairLayer.zIndex = 60;
            stack.push_back(hairLayer);
        }
    }

    // 7b. Face Details (Warpaint, Eyepatch)
    const CosmeticItem* faceDetailItem = m_equipment.GetEquipped(CosmeticCategory::Face);
    if (faceDetailItem && faceDetailItem->IsValid() && faceDetailItem->visible) {
        VisualLayer faceLayer;
        faceLayer.name = "face";
        faceLayer.category = CosmeticCategory::Face;
        faceLayer.spritesheetId = faceDetailItem->spritesheetId;
        faceLayer.textureId = faceDetailItem->textureId;
        faceLayer.tint = faceDetailItem->tint;
        faceLayer.anchorOffset = faceDetailItem->anchorOffset;
        faceLayer.zIndex = (facing == graphics::animation::Direction::Up) ? 13 : 62;
        faceLayer.visible = (facing != graphics::animation::Direction::Up);
        stack.push_back(faceLayer);
    }

    // 8. Mask Layer (lower face / cloth / leather / porcelain)
    const CosmeticItem* maskItem = m_equipment.GetEquipped(CosmeticCategory::Mask);
    if (maskItem && maskItem->IsValid() && maskItem->visible) {
        VisualLayer maskLayer;
        maskLayer.name = "mask";
        maskLayer.category = CosmeticCategory::Mask;
        maskLayer.spritesheetId = maskItem->spritesheetId;
        maskLayer.textureId = maskItem->textureId;
        maskLayer.tint = maskItem->tint;
        maskLayer.anchorOffset = maskItem->anchorOffset;
        maskLayer.zIndex = (facing == graphics::animation::Direction::Up) ? 14 : 68;
        maskLayer.visible = (facing != graphics::animation::Direction::Up);
        stack.push_back(maskLayer);
    }

    // 9. Goggles & Eyewear Layer
    bool hideGoggles = m_equipment.HasOcclusionFlag("mask_hide_goggles");
    if (!hideGoggles) {
        const CosmeticItem* gogglesItem = m_equipment.GetEquipped(CosmeticCategory::Goggles);
        if (gogglesItem && gogglesItem->IsValid() && gogglesItem->visible) {
            VisualLayer gogglesLayer;
            gogglesLayer.name = "goggles";
            gogglesLayer.category = gogglesItem->category;
            gogglesLayer.spritesheetId = gogglesItem->spritesheetId;
            gogglesLayer.textureId = gogglesItem->textureId;
            gogglesLayer.tint = gogglesItem->tint;
            gogglesLayer.anchorOffset = gogglesItem->anchorOffset;
            gogglesLayer.zIndex = (facing == graphics::animation::Direction::Up) ? 15 : 72;
            gogglesLayer.visible = (facing != graphics::animation::Direction::Up);
            stack.push_back(gogglesLayer);
        }
    }

    // 10. Headwear (Hats, Helmets)
    const CosmeticItem* headItem = m_equipment.GetEquipped(CosmeticCategory::Headwear);
    if (headItem && headItem->IsValid() && headItem->visible) {
        VisualLayer headLayer;
        headLayer.name = "headwear";
        headLayer.category = CosmeticCategory::Headwear;
        headLayer.spritesheetId = headItem->spritesheetId;
        headLayer.textureId = headItem->textureId;
        headLayer.tint = headItem->tint;
        headLayer.anchorOffset = headItem->anchorOffset;
        headLayer.zIndex = 80;
        stack.push_back(headLayer);
    }

    // Sort layers by ascending Z-Index
    std::sort(stack.begin(), stack.end(), [](const VisualLayer& a, const VisualLayer& b) {
        return a.zIndex < b.zIndex;
    });

    return stack;
}

void ModularCharacter::Render(graphics::Renderer& renderer,
                            resources::AssetManager& assetMgr,
                            Vector2 position,
                            graphics::animation::Direction facing,
                            const graphics::animation::AnimationController& animCtrl)
{
    // 1. Draw contact ground shadow
    if (m_renderShadow) {
        ::DrawEllipse(static_cast<int>(position.x), static_cast<int>(position.y - 1.0f),
                      9.0f, 4.0f, Color{ 12, 15, 29, 115 });
    }

    // 2. Shared animation frame query
    int frameIndex = animCtrl.GetSpritesheetFrameIndex();
    Rectangle baseSrcRect = animCtrl.GetCurrentFrameRect(assetMgr);
    Vector2 baseOrigin = animCtrl.GetCurrentFrameOrigin(assetMgr);

    // 3. Build directionally sorted layer stack
    auto layers = BuildLayerStack(facing);

    // 4. Render layers in Z-order
    for (const auto& layer : layers) {
        if (!layer.visible) continue;

        std::string sheetId = layer.spritesheetId.empty() ? layer.textureId : layer.spritesheetId;
        if (sheetId.empty()) continue;

        const auto* sheet = assetMgr.GetSpriteSheet(sheetId);

        Texture2D tex{ 0 };
        Rectangle srcRect = baseSrcRect;
        Vector2 origin = baseOrigin;

        if (sheet) {
            tex = assetMgr.GetTexture(sheet->GetTextureId());
            if (tex.id == 0) {
                tex = assetMgr.GetTexture(sheetId);
            }
            srcRect = sheet->GetFrameRect(frameIndex);
            origin = sheet->GetFrameOrigin(frameIndex);
        } else if (assetMgr.HasTexture(sheetId)) {
            tex = assetMgr.GetTexture(sheetId);
        } else {
            continue;
        }

        if (tex.id == 0) continue;

        Vector2 effectiveOrigin = { origin.x - layer.anchorOffset.x, origin.y - layer.anchorOffset.y };

        Rectangle destRect{
            position.x - effectiveOrigin.x,
            position.y - effectiveOrigin.y,
            srcRect.width,
            srcRect.height
        };

        renderer.DrawSprite(tex, srcRect, destRect, Vector2{ 0.0f, 0.0f }, 0.0f, layer.tint);
    }
}

nlohmann::json ModularCharacter::ToJson() const {
    nlohmann::json j;
    j["base_body"] = m_baseBodySpritesheet;
    j["render_shadow"] = m_renderShadow;

    nlohmann::json eqJson = nlohmann::json::object();
    for (const auto& [cat, item] : m_equipment.GetAllEquipped()) {
        eqJson[std::string(CosmeticCategoryToString(cat))] = item.ToJson();
    }
    j["equipment"] = eqJson;
    return j;
}

bool ModularCharacter::FromJson(const nlohmann::json& j, const resources::AssetManager& /*assetMgr*/) {
    m_baseBodySpritesheet = j.value("base_body", "player_spritesheet");
    m_renderShadow = j.value("render_shadow", true);

    m_equipment.Clear();
    if (j.contains("equipment") && j["equipment"].is_object()) {
        for (auto it = j["equipment"].begin(); it != j["equipment"].end(); ++it) {
            CosmeticItem item = CosmeticItem::FromJson(it.value());
            if (item.IsValid()) {
                m_equipment.Equip(item);
            }
        }
    }
    return true;
}

bool ModularCharacter::LoadAppearance(const std::string& jsonPath, const resources::AssetManager& assetMgr) {
    try {
        std::ifstream file(jsonPath);
        if (!file.is_open()) {
            Logger::Warn("MODULAR_CHAR", "Could not open appearance config: " + jsonPath);
            return false;
        }
        nlohmann::json j;
        file >> j;
        return FromJson(j, assetMgr);
    }
    catch (const std::exception& ex) {
        Logger::Error("MODULAR_CHAR", "Failed parsing appearance JSON '" + jsonPath + "': " + ex.what());
        return false;
    }
}

bool ModularCharacter::SaveAppearance(const std::string& jsonPath) const {
    try {
        std::ofstream file(jsonPath);
        if (!file.is_open()) {
            Logger::Error("MODULAR_CHAR", "Could not open file for saving appearance: " + jsonPath);
            return false;
        }
        nlohmann::json j = ToJson();
        file << j.dump(2);
        Logger::Info("MODULAR_CHAR", "Saved character appearance to: " + jsonPath);
        return true;
    }
    catch (const std::exception& ex) {
        Logger::Error("MODULAR_CHAR", "Failed saving appearance JSON: " + std::string(ex.what()));
        return false;
    }
}

void ModularCharacter::ApplyDefaultAppearance() {
    m_equipment.Clear();

    // Default Adventurer loadout
    CosmeticItem hair;
    hair.id = "hair_short_brown";
    hair.name = "Short Brown Hair";
    hair.category = CosmeticCategory::Hair;
    hair.spritesheetId = "sheet_hair_short_brown";
    m_equipment.Equip(hair);

    CosmeticItem shirt;
    shirt.id = "shirt_forest_tunic";
    shirt.name = "Forest Tunic";
    shirt.category = CosmeticCategory::Shirt;
    shirt.spritesheetId = "sheet_shirt_forest_tunic";
    m_equipment.Equip(shirt);

    CosmeticItem pants;
    pants.id = "pants_traveler_trousers";
    pants.name = "Traveler Trousers";
    pants.category = CosmeticCategory::Pants;
    pants.spritesheetId = "sheet_pants_traveler_trousers";
    m_equipment.Equip(pants);

    CosmeticItem shoes;
    shoes.id = "shoes_leather_boots";
    shoes.name = "Leather Boots";
    shoes.category = CosmeticCategory::Shoes;
    shoes.spritesheetId = "sheet_shoes_leather_boots";
    m_equipment.Equip(shoes);
}

} // namespace wildbound::entities


#include "CosmeticDefinition.hpp"

namespace wildbound::customization {

nlohmann::json CosmeticDefinition::ToJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["display_name"] = displayName;
    j["description"] = description;
    j["slot"] = std::string(CustomizationSlotToString(slot));
    j["category"] = category;
    j["spritesheet"] = spritesheetId;
    j["texture"] = textureId;
    j["icon"] = iconId;
    j["compatible_directions"] = compatibleDirections;
    j["compatible_animations"] = compatibleAnimations;
    j["occlusion_flags"] = occlusionFlags;
    j["anchor_offset"] = { anchorOffset.x, anchorOffset.y };
    j["tint"] = { tint.r, tint.g, tint.b, tint.a };
    j["rarity"] = rarity;
    j["is_gameplay_equipment"] = isGameplayEquipment;
    j["equipment_item_id"] = equipmentItemDefId;
    return j;
}

CosmeticDefinition CosmeticDefinition::FromJson(const nlohmann::json& j) {
    CosmeticDefinition def;
    if (j.contains("id") && j["id"].is_string()) {
        def.id = j["id"].get<std::string>();
    }
    if (j.contains("display_name") && j["display_name"].is_string()) {
        def.displayName = j["display_name"].get<std::string>();
    } else if (j.contains("name") && j["name"].is_string()) {
        def.displayName = j["name"].get<std::string>();
    }
    if (j.contains("description") && j["description"].is_string()) {
        def.description = j["description"].get<std::string>();
    }
    if (j.contains("slot") && j["slot"].is_string()) {
        def.slot = StringToCustomizationSlot(j["slot"].get<std::string>());
    } else if (j.contains("category") && j["category"].is_string()) {
        def.slot = StringToCustomizationSlot(j["category"].get<std::string>());
    }
    if (j.contains("category") && j["category"].is_string()) {
        def.category = j["category"].get<std::string>();
    }
    if (j.contains("spritesheet") && j["spritesheet"].is_string()) {
        def.spritesheetId = j["spritesheet"].get<std::string>();
    }
    if (j.contains("texture") && j["texture"].is_string()) {
        def.textureId = j["texture"].get<std::string>();
    }
    if (j.contains("icon") && j["icon"].is_string()) {
        def.iconId = j["icon"].get<std::string>();
    }
    if (j.contains("compatible_directions") && j["compatible_directions"].is_array()) {
        def.compatibleDirections.clear();
        for (const auto& d : j["compatible_directions"]) {
            if (d.is_string()) def.compatibleDirections.push_back(d.get<std::string>());
        }
    }
    if (j.contains("compatible_animations") && j["compatible_animations"].is_array()) {
        def.compatibleAnimations.clear();
        for (const auto& a : j["compatible_animations"]) {
            if (a.is_string()) def.compatibleAnimations.push_back(a.get<std::string>());
        }
    }
    if (j.contains("occlusion_flags") && j["occlusion_flags"].is_array()) {
        def.occlusionFlags.clear();
        for (const auto& f : j["occlusion_flags"]) {
            if (f.is_string()) def.occlusionFlags.push_back(f.get<std::string>());
        }
    }
    if (j.contains("anchor_offset") && j["anchor_offset"].is_array() && j["anchor_offset"].size() >= 2) {
        def.anchorOffset.x = j["anchor_offset"][0].get<float>();
        def.anchorOffset.y = j["anchor_offset"][1].get<float>();
    }
    if (j.contains("tint") && j["tint"].is_array() && j["tint"].size() >= 4) {
        def.tint.r = static_cast<unsigned char>(j["tint"][0].get<int>());
        def.tint.g = static_cast<unsigned char>(j["tint"][1].get<int>());
        def.tint.b = static_cast<unsigned char>(j["tint"][2].get<int>());
        def.tint.a = static_cast<unsigned char>(j["tint"][3].get<int>());
    }
    if (j.contains("rarity") && j["rarity"].is_string()) {
        def.rarity = j["rarity"].get<std::string>();
    }
    if (j.contains("is_gameplay_equipment") && j["is_gameplay_equipment"].is_boolean()) {
        def.isGameplayEquipment = j["is_gameplay_equipment"].get<bool>();
    }
    if (j.contains("equipment_item_id") && j["equipment_item_id"].is_string()) {
        def.equipmentItemDefId = j["equipment_item_id"].get<std::string>();
    }
    return def;
}

} // namespace wildbound::customization


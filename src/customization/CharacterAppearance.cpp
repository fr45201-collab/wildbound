#include "CharacterAppearance.hpp"
#include "CosmeticRegistry.hpp"
#include "../entities/ModularCharacter.hpp"
#include "../core/Logger.hpp"

namespace wildbound::customization {

CharacterAppearance::CharacterAppearance() {
    ResetToDefault();
}

void CharacterAppearance::SetCosmetic(CustomizationSlot slot, const std::string& cosmeticId) {
    if (cosmeticId.empty()) {
        ClearSlot(slot);
    } else {
        m_slots[slot] = cosmeticId;
    }
}

std::string CharacterAppearance::GetCosmetic(CustomizationSlot slot) const {
    auto it = m_slots.find(slot);
    if (it != m_slots.end()) {
        return it->second;
    }
    return "";
}

bool CharacterAppearance::HasCosmetic(CustomizationSlot slot) const {
    auto it = m_slots.find(slot);
    return it != m_slots.end() && !it->second.empty();
}

void CharacterAppearance::ClearSlot(CustomizationSlot slot) {
    m_slots.erase(slot);
}

void CharacterAppearance::ClearAll() {
    m_slots.clear();
}

void CharacterAppearance::ResetToDefault() {
    m_slots.clear();
    m_baseBodySheet = "sheet_skin_default";
    m_slots[CustomizationSlot::Skin] = "skin_default";
    m_slots[CustomizationSlot::Hair] = "hair_short_brown";
    m_slots[CustomizationSlot::Chest] = "shirt_forest_tunic";
    m_slots[CustomizationSlot::Legs] = "pants_traveler_trousers";
    m_slots[CustomizationSlot::Feet] = "shoes_leather_boots";
    m_slots[CustomizationSlot::Back] = "back_traveler_backpack";
}

void CharacterAppearance::ApplyToModularCharacter(entities::ModularCharacter& modularChar, const CosmeticRegistry& registry) const {
    auto& equip = modularChar.GetEquipment();
    equip.Clear();

    modularChar.SetBaseBody(m_baseBodySheet);

    for (const auto& [slot, cosmeticId] : m_slots) {
        if (cosmeticId.empty()) continue;

        const auto* def = registry.GetCosmetic(cosmeticId);
        if (def) {
            equip.Equip(def->ToCosmeticItem());
        } else {
            // Fallback: create item directly from ID if registered in ModularCharacter
            entities::CosmeticItem fallbackItem;
            fallbackItem.id = cosmeticId;
            fallbackItem.name = cosmeticId;
            fallbackItem.category = CustomizationSlotToCategory(slot);
            fallbackItem.spritesheetId = "sheet_" + cosmeticId;
            fallbackItem.textureId = "tex_" + cosmeticId;
            equip.Equip(fallbackItem);
        }
    }
}

void CharacterAppearance::SyncFromModularCharacter(const entities::ModularCharacter& modularChar) {
    m_slots.clear();
    m_baseBodySheet = modularChar.GetBaseBody();

    const auto& equip = modularChar.GetEquipment();
    for (const auto& [cat, item] : equip.GetAllEquipped()) {
        CustomizationSlot slot = CategoryToCustomizationSlot(cat);
        m_slots[slot] = item.id;
    }
}

nlohmann::json CharacterAppearance::ToJson() const {
    nlohmann::json j;
    j["base_body"] = m_baseBodySheet;
    nlohmann::json slotsJson = nlohmann::json::object();
    for (const auto& [slot, cosmeticId] : m_slots) {
        slotsJson[std::string(CustomizationSlotToString(slot))] = cosmeticId;
    }
    j["slots"] = slotsJson;
    return j;
}

bool CharacterAppearance::FromJson(const nlohmann::json& j) {
    if (!j.is_object()) return false;

    if (j.contains("base_body") && j["base_body"].is_string()) {
        m_baseBodySheet = j["base_body"].get<std::string>();
    }

    if (j.contains("slots") && j["slots"].is_object()) {
        m_slots.clear();
        for (auto it = j["slots"].begin(); it != j["slots"].end(); ++it) {
            CustomizationSlot slot = StringToCustomizationSlot(it.key());
            if (it.value().is_string()) {
                m_slots[slot] = it.value().get<std::string>();
            }
        }
        return true;
    }

    // Support legacy schema with "equipment" object (e.g. data/characters/player_appearance.json)
    if (j.contains("equipment") && j["equipment"].is_object()) {
        m_slots.clear();
        for (auto it = j["equipment"].begin(); it != j["equipment"].end(); ++it) {
            CustomizationSlot slot = StringToCustomizationSlot(it.key());
            if (it.value().is_object() && it.value().contains("id") && it.value()["id"].is_string()) {
                m_slots[slot] = it.value()["id"].get<std::string>();
            }
        }
        return true;
    }

    return false;
}

} // namespace wildbound::customization


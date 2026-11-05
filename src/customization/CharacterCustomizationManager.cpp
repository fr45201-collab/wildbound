#include "CharacterCustomizationManager.hpp"
#include "../entities/Player.hpp"
#include "../entities/EquipmentSystem.hpp"
#include "../resources/AssetManager.hpp"
#include "../cosmetics/CosmeticsOwnershipManager.hpp"
#include "../core/Logger.hpp"

#include <chrono>

namespace wildbound::customization {

CharacterCustomizationManager::CharacterCustomizationManager()
    : m_savePath(AppearanceSerializer::DEFAULT_LOADOUT_FILE_PATH)
{
}

bool CharacterCustomizationManager::Initialize(const resources::AssetManager& assetMgr,
                                              const std::string& catalogPath,
                                              const std::string& loadoutSavePath) {
    m_savePath = loadoutSavePath;
    m_registry.Clear();

    // 1. Register builtin cosmetics (Phase 7 core and Phase 8 production cosmetics)
    m_registry.RegisterBuiltinCosmetics(&assetMgr.GetRegistry());

    // 2. Load Phase 17 catalog if present
    m_registry.LoadFromCatalog(catalogPath, &assetMgr.GetRegistry());

    // 3. Load persisted loadouts and active appearance
    LoadLoadouts();

    m_initialized = true;
    core::Logger::Info("CUSTOMIZATION_MGR", "CharacterCustomizationManager initialized with " +
                       std::to_string(m_registry.GetCount()) + " cosmetics and " +
                       std::to_string(m_loadouts.size()) + " loadouts.");
    return true;
}

void CharacterCustomizationManager::SetPlayer(entities::Player* player) {
    m_player = player;
    if (m_player) {
        SynchronizePlayerAppearance();
    }
}

void CharacterCustomizationManager::BeginPreview() {
    m_preview.BeginPreview(m_currentAppearance, m_registry);
}

bool CharacterCustomizationManager::ApplyPreviewCosmetic(const std::string& cosmeticId) {
    const auto* def = m_registry.GetCosmetic(cosmeticId);
    if (!def) {
        core::Logger::Warn("CUSTOMIZATION_MGR", "Unknown cosmetic ID for preview: " + cosmeticId);
        return false;
    }
    return m_preview.ApplyPreviewCosmetic(*def, m_registry);
}

bool CharacterCustomizationManager::UnequipPreviewSlot(CustomizationSlot slot) {
    return m_preview.UnequipPreviewSlot(slot, m_registry);
}

void CharacterCustomizationManager::ResetPreview() {
    m_preview.ResetPreview(m_registry);
}

bool CharacterCustomizationManager::ConfirmPreview() {
    if (!m_preview.IsActive()) return false;

    bool confirmed = m_preview.ConfirmPreview(m_currentAppearance);
    if (confirmed) {
        SynchronizePlayerAppearance();
        SaveLoadouts();
        core::Logger::Info("CUSTOMIZATION_MGR", "Character appearance preview confirmed and applied to player.");
    }
    return confirmed;
}

void CharacterCustomizationManager::CancelPreview() {
    m_preview.CancelPreview();
    core::Logger::Info("CUSTOMIZATION_MGR", "Character appearance preview cancelled.");
}

bool CharacterCustomizationManager::EquipCosmetic(const std::string& cosmeticId) {
    const auto* def = m_registry.GetCosmetic(cosmeticId);
    if (!def) {
        core::Logger::Warn("CUSTOMIZATION_MGR", "Cannot equip unknown cosmetic ID: " + cosmeticId);
        return false;
    }

    if (m_ownershipMgr && !m_ownershipMgr->IsOwned(cosmeticId)) {
        core::Logger::Warn("CUSTOMIZATION_MGR", "Cannot equip locked cosmetic: " + cosmeticId);
        return false;
    }

    std::string reason;
    if (!ValidateCosmeticCompatibility(cosmeticId, &reason)) {
        core::Logger::Warn("CUSTOMIZATION_MGR", "Cosmetic compatibility validation failed: " + reason);
        return false;
    }

    m_currentAppearance.SetCosmetic(def->slot, cosmeticId);
    SynchronizePlayerAppearance();
    SaveLoadouts();
    return true;
}

bool CharacterCustomizationManager::UnequipCosmetic(CustomizationSlot slot) {
    m_currentAppearance.ClearSlot(slot);
    SynchronizePlayerAppearance();
    SaveLoadouts();
    return true;
}

bool CharacterCustomizationManager::ReplaceCosmetic(CustomizationSlot slot, const std::string& cosmeticId) {
    const auto* def = m_registry.GetCosmetic(cosmeticId);
    if (!def || def->slot != slot) return false;
    return EquipCosmetic(cosmeticId);
}

void CharacterCustomizationManager::ResetCosmeticSlot(CustomizationSlot slot) {
    CharacterAppearance defaultApp;
    defaultApp.ResetToDefault();
    m_currentAppearance.SetCosmetic(slot, defaultApp.GetCosmetic(slot));
    SynchronizePlayerAppearance();
    SaveLoadouts();
}

void CharacterCustomizationManager::ResetToDefault() {
    m_currentAppearance.ResetToDefault();
    SynchronizePlayerAppearance();
    SaveLoadouts();
}

std::string CharacterCustomizationManager::GetEquippedCosmetic(CustomizationSlot slot) const {
    return m_currentAppearance.GetCosmetic(slot);
}

std::vector<const CosmeticDefinition*> CharacterCustomizationManager::GetAvailableCosmetics(CustomizationSlot slot) const {
    return m_registry.GetCosmeticsBySlot(slot);
}

bool CharacterCustomizationManager::ValidateCosmeticCompatibility(const std::string& cosmeticId, std::string* outReason) const {
    const auto* def = m_registry.GetCosmetic(cosmeticId);
    if (!def) {
        if (outReason) *outReason = "Cosmetic not found in registry.";
        return false;
    }

    // Directional check: must support basic directions
    if (!def->SupportsDirection(graphics::animation::Direction::Down)) {
        if (outReason) *outReason = "Cosmetic does not support forward (down) direction.";
        return false;
    }

    return true;
}

bool CharacterCustomizationManager::CreateLoadout(const std::string& name) {
    if (m_loadouts.size() >= m_maxLoadouts) {
        core::Logger::Warn("CUSTOMIZATION_MGR", "Cannot create loadout: reached maximum capacity of " +
                           std::to_string(m_maxLoadouts));
        return false;
    }

    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    AppearanceLoadout l;
    l.id = "loadout_" + std::to_string(now);
    l.name = name.empty() ? ("Loadout " + std::to_string(m_loadouts.size() + 1)) : name;
    l.appearance = m_currentAppearance;
    l.timestamp = now;

    m_loadouts.push_back(l);
    SaveLoadouts();
    return true;
}

bool CharacterCustomizationManager::SaveCurrentToLoadout(size_t loadoutIndex) {
    if (loadoutIndex >= m_loadouts.size()) return false;

    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    m_loadouts[loadoutIndex].appearance = m_currentAppearance;
    m_loadouts[loadoutIndex].timestamp = now;
    SaveLoadouts();
    return true;
}

bool CharacterCustomizationManager::RenameLoadout(size_t loadoutIndex, const std::string& newName) {
    if (loadoutIndex >= m_loadouts.size() || newName.empty()) return false;

    m_loadouts[loadoutIndex].name = newName;
    SaveLoadouts();
    return true;
}

bool CharacterCustomizationManager::LoadLoadout(size_t loadoutIndex) {
    if (loadoutIndex >= m_loadouts.size()) return false;

    CharacterAppearance validatedApp = m_loadouts[loadoutIndex].appearance;
    if (m_ownershipMgr) {
        for (uint8_t i = 0; i < static_cast<uint8_t>(CustomizationSlot::Count); ++i) {
            auto slot = static_cast<CustomizationSlot>(i);
            std::string id = validatedApp.GetCosmetic(slot);
            if (!id.empty() && !m_ownershipMgr->IsOwned(id)) {
                core::Logger::Warn("CUSTOMIZATION_MGR", "Loadout cosmetic locked: " + id + ", clearing slot.");
                validatedApp.ClearSlot(slot);
            }
        }
    }

    m_currentAppearance = validatedApp;
    SynchronizePlayerAppearance();
    SaveLoadouts();

    if (m_preview.IsActive()) {
        m_preview.BeginPreview(m_currentAppearance, m_registry);
    }
    return true;
}

bool CharacterCustomizationManager::DeleteLoadout(size_t loadoutIndex) {
    if (loadoutIndex >= m_loadouts.size()) return false;

    m_loadouts.erase(m_loadouts.begin() + loadoutIndex);
    SaveLoadouts();
    return true;
}

bool CharacterCustomizationManager::DuplicateLoadout(size_t loadoutIndex) {
    if (loadoutIndex >= m_loadouts.size()) return false;
    if (m_loadouts.size() >= m_maxLoadouts) return false;

    AppearanceLoadout copy = m_loadouts[loadoutIndex];
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    copy.id = "loadout_" + std::to_string(now);
    copy.name = copy.name + " (Copy)";
    copy.timestamp = now;

    m_loadouts.push_back(copy);
    SaveLoadouts();
    return true;
}

bool CharacterCustomizationManager::SaveLoadouts() {
    return AppearanceSerializer::SaveLoadouts(m_savePath, m_loadouts, m_currentAppearance);
}

bool CharacterCustomizationManager::LoadLoadouts() {
    return AppearanceSerializer::LoadLoadouts(m_savePath, m_loadouts, m_currentAppearance);
}

void CharacterCustomizationManager::OnEquipmentChanged(entities::EquipmentSlot slot, const std::string& itemDefId) {
    // Synchronize gameplay equipment visually without altering combat stats
    CustomizationSlot custSlot = CustomizationSlot::Chest;
    switch (slot) {
        case entities::EquipmentSlot::Head: custSlot = CustomizationSlot::Headwear; break;
        case entities::EquipmentSlot::Chest: custSlot = CustomizationSlot::Chest; break;
        case entities::EquipmentSlot::Legs: custSlot = CustomizationSlot::Legs; break;
        case entities::EquipmentSlot::Feet: custSlot = CustomizationSlot::Feet; break;
        case entities::EquipmentSlot::Face: custSlot = CustomizationSlot::Face; break;
        case entities::EquipmentSlot::Back: custSlot = CustomizationSlot::Back; break;
        default: return; // Weapons are handled separately by ModularCharacter weapons
    }

    if (itemDefId.empty()) {
        // Unequipped: restore default cosmetic for that slot
        CharacterAppearance defApp;
        defApp.ResetToDefault();
        m_currentAppearance.SetCosmetic(custSlot, defApp.GetCosmetic(custSlot));
    } else {
        // Find if item has a corresponding cosmetic ID
        if (m_registry.HasCosmetic(itemDefId)) {
            m_currentAppearance.SetCosmetic(custSlot, itemDefId);
        } else if (m_registry.HasCosmetic("arm_" + itemDefId)) {
            m_currentAppearance.SetCosmetic(custSlot, "arm_" + itemDefId);
        } else if (m_registry.HasCosmetic("clo_" + itemDefId)) {
            m_currentAppearance.SetCosmetic(custSlot, "clo_" + itemDefId);
        }
    }

    SynchronizePlayerAppearance();
}

void CharacterCustomizationManager::SynchronizePlayerAppearance() {
    if (!m_player) return;

    m_currentAppearance.ApplyToModularCharacter(m_player->GetModularAppearance(), m_registry);
}

} // namespace wildbound::customization


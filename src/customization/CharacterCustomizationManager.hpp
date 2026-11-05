#pragma once

#include "CosmeticTypes.hpp"
#include "CosmeticDefinition.hpp"
#include "CosmeticRegistry.hpp"
#include "CharacterAppearance.hpp"
#include "AppearancePreview.hpp"
#include "AppearanceLoadout.hpp"
#include "AppearanceSerializer.hpp"

#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace wildbound::entities {
    class Player;
    enum class EquipmentSlot;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::cosmetics {
    class CosmeticsOwnershipManager;
}

namespace wildbound::customization {

class CharacterCustomizationManager {
public:
    static constexpr size_t DEFAULT_MAX_LOADOUTS = 5;

    CharacterCustomizationManager();
    ~CharacterCustomizationManager() = default;

    // Initialization
    bool Initialize(const resources::AssetManager& assetMgr,
                    const std::string& catalogPath = "data/cosmetics/costume_catalog.json",
                    const std::string& loadoutSavePath = AppearanceSerializer::DEFAULT_LOADOUT_FILE_PATH);

    void SetPlayer(entities::Player* player);
    [[nodiscard]] entities::Player* GetPlayer() const noexcept { return m_player; }

    // Ownership Management Integration
    void SetOwnershipManager(cosmetics::CosmeticsOwnershipManager* mgr) noexcept { m_ownershipMgr = mgr; }
    [[nodiscard]] cosmetics::CosmeticsOwnershipManager* GetOwnershipManager() const noexcept { return m_ownershipMgr; }

    // Registry Access
    [[nodiscard]] CosmeticRegistry& GetRegistry() noexcept { return m_registry; }
    [[nodiscard]] const CosmeticRegistry& GetRegistry() const noexcept { return m_registry; }

    // Authoritative Appearance
    [[nodiscard]] const CharacterAppearance& GetCurrentAppearance() const noexcept { return m_currentAppearance; }
    [[nodiscard]] CharacterAppearance& GetCurrentAppearance() noexcept { return m_currentAppearance; }

    // Live Preview
    [[nodiscard]] AppearancePreview& GetPreview() noexcept { return m_preview; }
    [[nodiscard]] const AppearancePreview& GetPreview() const noexcept { return m_preview; }

    void BeginPreview();
    bool ApplyPreviewCosmetic(const std::string& cosmeticId);
    bool UnequipPreviewSlot(CustomizationSlot slot);
    void ResetPreview();
    bool ConfirmPreview();
    void CancelPreview();

    // Direct Authoritative Cosmetic Operations
    bool EquipCosmetic(const std::string& cosmeticId);
    bool UnequipCosmetic(CustomizationSlot slot);
    bool ReplaceCosmetic(CustomizationSlot slot, const std::string& cosmeticId);
    void ResetCosmeticSlot(CustomizationSlot slot);
    void ResetToDefault();

    [[nodiscard]] std::string GetEquippedCosmetic(CustomizationSlot slot) const;
    [[nodiscard]] std::vector<const CosmeticDefinition*> GetAvailableCosmetics(CustomizationSlot slot) const;
    [[nodiscard]] bool ValidateCosmeticCompatibility(const std::string& cosmeticId, std::string* outReason = nullptr) const;

    // Loadout Operations
    [[nodiscard]] const std::vector<AppearanceLoadout>& GetLoadouts() const noexcept { return m_loadouts; }
    [[nodiscard]] size_t GetMaxLoadouts() const noexcept { return m_maxLoadouts; }
    void SetMaxLoadouts(size_t maxCount) { m_maxLoadouts = maxCount; }

    bool CreateLoadout(const std::string& name);
    bool SaveCurrentToLoadout(size_t loadoutIndex);
    bool RenameLoadout(size_t loadoutIndex, const std::string& newName);
    bool LoadLoadout(size_t loadoutIndex);
    bool DeleteLoadout(size_t loadoutIndex);
    bool DuplicateLoadout(size_t loadoutIndex);

    // Persistence
    bool SaveLoadouts();
    bool LoadLoadouts();

    // Equipment System Integration
    void OnEquipmentChanged(entities::EquipmentSlot slot, const std::string& itemDefId);

    // Apply authoritative appearance to player
    void SynchronizePlayerAppearance();

private:
    CosmeticRegistry m_registry;
    CharacterAppearance m_currentAppearance;
    AppearancePreview m_preview;
    std::vector<AppearanceLoadout> m_loadouts;
    size_t m_maxLoadouts = DEFAULT_MAX_LOADOUTS;

    entities::Player* m_player = nullptr;
    cosmetics::CosmeticsOwnershipManager* m_ownershipMgr = nullptr;
    std::string m_savePath;
    bool m_initialized = false;
};

} // namespace wildbound::customization


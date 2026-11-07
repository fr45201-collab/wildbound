#pragma once

#include "CosmeticsRarity.hpp"
#include "CosmeticsColorVariant.hpp"
#include "CosmeticsOwnershipManager.hpp"
#include "CosmeticsSearchFilter.hpp"
#include "../customization/CosmeticDefinition.hpp"
#include "../customization/CosmeticTypes.hpp"

#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::network::http {
    class BackendClient;
}

namespace wildbound::customization {
    class CharacterCustomizationManager;
    class AppearancePreview;
}

namespace wildbound::cosmetics {

enum class StoreTab : uint8_t {
    Store = 0,
    Wardrobe = 1
};

inline std::string_view StoreTabToString(StoreTab tab) noexcept {
    switch (tab) {
        case StoreTab::Store:    return "Store";
        case StoreTab::Wardrobe: return "Wardrobe";
        default:                 return "Store";
    }
}

class CosmeticsStoreManager {
public:
    CosmeticsStoreManager();
    ~CosmeticsStoreManager() = default;

    bool Initialize(customization::CharacterCustomizationManager& customMgr,
                    audio::AudioSystem* audioSystem = nullptr,
                    const std::string& ownershipPath = CosmeticsOwnershipManager::DEFAULT_SAVE_PATH);

    void Update(float dt);

    // Tab and Navigation
    void SetActiveTab(StoreTab tab);
    [[nodiscard]] StoreTab GetActiveTab() const noexcept { return m_activeTab; }

    void SetCategory(const std::string& category);
    [[nodiscard]] const std::string& GetCategory() const noexcept { return m_activeCategory; }
    [[nodiscard]] static const std::vector<std::string>& GetStandardCategories() noexcept;

    // Search and Filtering
    CosmeticFilterCriteria& GetFilterCriteria() noexcept { return m_filterCriteria; }
    [[nodiscard]] const CosmeticFilterCriteria& GetFilterCriteria() const noexcept { return m_filterCriteria; }
    void SetSearchQuery(const std::string& query);
    void SetRarityFilter(std::optional<CosmeticRarity> rarity);
    void SetOwnershipFilter(OwnershipFilter filter);
    void SetSortOption(CosmeticSortOption sort);
    void ResetFilters();

    [[nodiscard]] std::vector<const customization::CosmeticDefinition*> GetDisplayedCosmetics() const;

    // Item Selection
    void SelectCosmetic(const std::string& cosmeticId);
    [[nodiscard]] const customization::CosmeticDefinition* GetSelectedCosmetic() const;
    [[nodiscard]] const std::string& GetSelectedCosmeticId() const noexcept { return m_selectedCosmeticId; }

    // Color Variations
    void SelectVariant(const std::string& variantId);
    [[nodiscard]] const std::string& GetSelectedVariantId() const noexcept { return m_selectedVariantId; }
    [[nodiscard]] const std::vector<CosmeticColorVariant>& GetAvailableVariants() const noexcept { return m_availableVariants; }
    [[nodiscard]] const CosmeticColorVariant* GetCurrentVariant() const;

    // Queries on selected cosmetic
    [[nodiscard]] bool IsSelectedOwned() const;
    [[nodiscard]] bool IsSelectedEquipped() const;

    // Actions
    bool EquipSelected();
    bool UnequipSlot(customization::CustomizationSlot slot);
    bool UnlockPrototypeSelected();
    bool PreviewSelected();

    // Preview Controls
    void RotatePreviewLeft();
    void RotatePreviewRight();
    void TogglePreviewAnimation();
    void ResetPreviewAppearance();

    // Loadout validation & application
    bool ApplyLoadoutValidated(size_t loadoutIndex);

    // Audio triggers
    void PlayAudioEvent(const std::string& eventName);

    // Accessors
    [[nodiscard]] CosmeticsOwnershipManager& GetOwnershipManager() noexcept { return m_ownershipMgr; }
    [[nodiscard]] const CosmeticsOwnershipManager& GetOwnershipManager() const noexcept { return m_ownershipMgr; }
    [[nodiscard]] customization::CharacterCustomizationManager* GetCustomizationManager() const noexcept { return m_customizationMgr; }
    void SetBackendClient(network::http::BackendClient* client) noexcept { m_backendClient = client; m_ownershipMgr.SetBackendClient(client); }
    [[nodiscard]] network::http::BackendClient* GetBackendClient() const noexcept { return m_backendClient; }

private:
    void RefreshColorVariants();
    void AutoSelectFirstAvailable();

    StoreTab m_activeTab = StoreTab::Store;
    std::string m_activeCategory = "all";
    CosmeticFilterCriteria m_filterCriteria;

    std::string m_selectedCosmeticId;
    std::string m_selectedVariantId = "variant_default";
    std::vector<CosmeticColorVariant> m_availableVariants;

    CosmeticsOwnershipManager m_ownershipMgr;
    customization::CharacterCustomizationManager* m_customizationMgr = nullptr;
    network::http::BackendClient* m_backendClient = nullptr;
    audio::AudioSystem* m_audioSystem = nullptr;
    bool m_initialized = false;
};

} // namespace wildbound::cosmetics


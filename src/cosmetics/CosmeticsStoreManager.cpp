#include "CosmeticsStoreManager.hpp"
#include "../customization/CharacterCustomizationManager.hpp"
#include "../audio/AudioSystem.hpp"
#include "../core/Logger.hpp"

#include <algorithm>

namespace wildbound::cosmetics {

namespace {

const std::vector<std::string> CATEGORIES = {
    "featured",
    "all",
    "hair",
    "clothing",
    "armor",
    "headwear",
    "face",
    "backpacks",
    "footwear",
    "special"
};

} // anonymous namespace

CosmeticsStoreManager::CosmeticsStoreManager() = default;

const std::vector<std::string>& CosmeticsStoreManager::GetStandardCategories() noexcept {
    return CATEGORIES;
}

bool CosmeticsStoreManager::Initialize(customization::CharacterCustomizationManager& customMgr,
                                       audio::AudioSystem* audioSystem,
                                       const std::string& ownershipPath) {
    m_customizationMgr = &customMgr;
    m_audioSystem = audioSystem;

    if (!m_ownershipMgr.Initialize(ownershipPath)) {
        core::Logger::Warn("COSMETICS_STORE", "Failed to initialize ownership manager from " + ownershipPath);
    }

    // Register all cosmetics from catalog into ownership manager
    const auto allCosmetics = m_customizationMgr->GetRegistry().GetAllCosmetics();
    for (const auto* def : allCosmetics) {
        if (def) {
            m_ownershipMgr.RegisterCosmetic(def->id, false);
        }
    }

    // Connect ownership manager to customization manager
    m_customizationMgr->SetOwnershipManager(&m_ownershipMgr);

    // Initialize preview mode
    m_customizationMgr->BeginPreview();

    // Select first available item
    AutoSelectFirstAvailable();

    m_initialized = true;
    core::Logger::Info("COSMETICS_STORE", "CosmeticsStoreManager initialized successfully.");
    return true;
}

void CosmeticsStoreManager::Update(float dt) {
    if (m_customizationMgr && m_customizationMgr->GetPreview().IsActive()) {
        m_customizationMgr->GetPreview().Update(dt);
    }
}

void CosmeticsStoreManager::SetActiveTab(StoreTab tab) {
    if (m_activeTab != tab) {
        m_activeTab = tab;
        if (m_activeTab == StoreTab::Wardrobe) {
            m_filterCriteria.sortOption = CosmeticSortOption::OwnedFirst;
        } else {
            m_filterCriteria.sortOption = CosmeticSortOption::RarityDesc;
        }
        AutoSelectFirstAvailable();
        PlayAudioEvent("cosmetics.tab.switch");
    }
}

void CosmeticsStoreManager::SetCategory(const std::string& category) {
    m_activeCategory = category;
    m_filterCriteria.category = category;
    AutoSelectFirstAvailable();
    PlayAudioEvent("cosmetics.category.select");
}

void CosmeticsStoreManager::SetSearchQuery(const std::string& query) {
    m_filterCriteria.searchQuery = query;
    AutoSelectFirstAvailable();
}

void CosmeticsStoreManager::SetRarityFilter(std::optional<CosmeticRarity> rarity) {
    m_filterCriteria.rarity = rarity;
    AutoSelectFirstAvailable();
}

void CosmeticsStoreManager::SetOwnershipFilter(OwnershipFilter filter) {
    m_filterCriteria.ownership = filter;
    AutoSelectFirstAvailable();
}

void CosmeticsStoreManager::SetSortOption(CosmeticSortOption sort) {
    m_filterCriteria.sortOption = sort;
}

void CosmeticsStoreManager::ResetFilters() {
    m_filterCriteria.Reset();
    m_activeCategory = "all";
    AutoSelectFirstAvailable();
}

std::vector<const customization::CosmeticDefinition*> CosmeticsStoreManager::GetDisplayedCosmetics() const {
    if (!m_customizationMgr) return {};

    const auto allCosmetics = m_customizationMgr->GetRegistry().GetAllCosmetics();
    return CosmeticsSearchFilter::FilterAndSort(allCosmetics, m_filterCriteria, m_ownershipMgr);
}

void CosmeticsStoreManager::SelectCosmetic(const std::string& cosmeticId) {
    if (m_selectedCosmeticId == cosmeticId) return;

    m_selectedCosmeticId = cosmeticId;
    RefreshColorVariants();

    std::string savedVariant = m_ownershipMgr.GetSelectedVariant(cosmeticId);
    if (!savedVariant.empty()) {
        m_selectedVariantId = savedVariant;
    } else if (!m_availableVariants.empty()) {
        m_selectedVariantId = m_availableVariants.front().id;
    }

    // Auto-preview selected cosmetic on character
    PreviewSelected();

    PlayAudioEvent("cosmetics.item.select");
}

const customization::CosmeticDefinition* CosmeticsStoreManager::GetSelectedCosmetic() const {
    if (!m_customizationMgr || m_selectedCosmeticId.empty()) return nullptr;
    return m_customizationMgr->GetRegistry().GetCosmetic(m_selectedCosmeticId);
}

void CosmeticsStoreManager::RefreshColorVariants() {
    m_availableVariants.clear();
    const auto* def = GetSelectedCosmetic();
    if (!def) return;

    m_availableVariants = ColorVariantPalette::GetVariantsForCategory(def->category);
}

void CosmeticsStoreManager::SelectVariant(const std::string& variantId) {
    m_selectedVariantId = variantId;
    if (IsSelectedOwned()) {
        m_ownershipMgr.SetSelectedVariant(m_selectedCosmeticId, variantId);
    }
    PlayAudioEvent("cosmetics.variant.select");
}

const CosmeticColorVariant* CosmeticsStoreManager::GetCurrentVariant() const {
    for (const auto& v : m_availableVariants) {
        if (v.id == m_selectedVariantId) {
            return &v;
        }
    }
    if (!m_availableVariants.empty()) {
        return &m_availableVariants.front();
    }
    return nullptr;
}

bool CosmeticsStoreManager::IsSelectedOwned() const {
    if (m_selectedCosmeticId.empty()) return false;
    return m_ownershipMgr.IsOwned(m_selectedCosmeticId);
}

bool CosmeticsStoreManager::IsSelectedEquipped() const {
    if (!m_customizationMgr || m_selectedCosmeticId.empty()) return false;
    const auto* def = GetSelectedCosmetic();
    if (!def) return false;

    return m_customizationMgr->GetCurrentAppearance().GetCosmetic(def->slot) == m_selectedCosmeticId;
}

bool CosmeticsStoreManager::EquipSelected() {
    if (!m_customizationMgr || m_selectedCosmeticId.empty()) return false;

    if (!IsSelectedOwned()) {
        core::Logger::Warn("COSMETICS_STORE", "Cannot equip locked cosmetic: " + m_selectedCosmeticId);
        PlayAudioEvent("cosmetics.error");
        return false;
    }

    bool success = m_customizationMgr->EquipCosmetic(m_selectedCosmeticId);
    if (success) {
        PlayAudioEvent("cosmetics.equip");
    }
    return success;
}

bool CosmeticsStoreManager::UnequipSlot(customization::CustomizationSlot slot) {
    if (!m_customizationMgr) return false;
    bool success = m_customizationMgr->UnequipCosmetic(slot);
    if (success) {
        PlayAudioEvent("cosmetics.unequip");
    }
    return success;
}

bool CosmeticsStoreManager::UnlockPrototypeSelected() {
    if (m_selectedCosmeticId.empty()) return false;
    if (IsSelectedOwned()) return true;

    bool success = m_ownershipMgr.UnlockPrototype(m_selectedCosmeticId, "prototype_unlock");
    if (success) {
        PlayAudioEvent("cosmetics.unlock.prototype");
        core::Logger::Info("COSMETICS_STORE", "Cosmetic unlocked (prototype): " + m_selectedCosmeticId);
    }
    return success;
}

bool CosmeticsStoreManager::PreviewSelected() {
    if (!m_customizationMgr || m_selectedCosmeticId.empty()) return false;

    bool success = m_customizationMgr->ApplyPreviewCosmetic(m_selectedCosmeticId);
    if (success) {
        PlayAudioEvent("cosmetics.item.preview");
    }
    return success;
}

void CosmeticsStoreManager::RotatePreviewLeft() {
    if (m_customizationMgr) {
        m_customizationMgr->GetPreview().RotateCounterClockwise();
    }
}

void CosmeticsStoreManager::RotatePreviewRight() {
    if (m_customizationMgr) {
        m_customizationMgr->GetPreview().RotateClockwise();
    }
}

void CosmeticsStoreManager::TogglePreviewAnimation() {
    if (m_customizationMgr) {
        m_customizationMgr->GetPreview().ToggleAnimationMode();
    }
}

void CosmeticsStoreManager::ResetPreviewAppearance() {
    if (m_customizationMgr) {
        m_customizationMgr->ResetPreview();
    }
}

bool CosmeticsStoreManager::ApplyLoadoutValidated(size_t loadoutIndex) {
    if (!m_customizationMgr) return false;
    const auto& loadouts = m_customizationMgr->GetLoadouts();
    if (loadoutIndex >= loadouts.size()) return false;

    const auto& loadout = loadouts[loadoutIndex];
    customization::CharacterAppearance validatedAppearance = loadout.appearance;

    // Validate every slot against ownership
    for (uint8_t i = 0; i < static_cast<uint8_t>(customization::CustomizationSlot::Count); ++i) {
        auto slot = static_cast<customization::CustomizationSlot>(i);
        std::string cosmeticId = validatedAppearance.GetCosmetic(slot);
        if (!cosmeticId.empty() && !m_ownershipMgr.IsOwned(cosmeticId)) {
            core::Logger::Warn("COSMETICS_STORE", "Loadout contains unowned cosmetic " + cosmeticId + " in slot " +
                               std::string(customization::CustomizationSlotToString(slot)) + ". Resetting slot to default.");
            validatedAppearance.ClearSlot(slot);
        }
    }

    m_customizationMgr->GetCurrentAppearance() = validatedAppearance;
    m_customizationMgr->SynchronizePlayerAppearance();
    m_customizationMgr->SaveLoadouts();
    PlayAudioEvent("cosmetics.loadout.apply");
    return true;
}

void CosmeticsStoreManager::PlayAudioEvent(const std::string& eventName) {
    if (!m_audioSystem) return;
    // Map cosmetic audio event to audio sound effect if mapped
    if (eventName == "cosmetics.equip") {
        m_audioSystem->PlaySound("ui_equip");
    } else if (eventName == "cosmetics.unequip") {
        m_audioSystem->PlaySound("ui_unequip");
    } else if (eventName == "cosmetics.unlock.prototype") {
        m_audioSystem->PlaySound("ui_quest_complete");
    } else if (eventName == "cosmetics.item.select") {
        m_audioSystem->PlaySound("ui_click");
    } else {
        m_audioSystem->PlaySound("ui_hover");
    }
}

void CosmeticsStoreManager::AutoSelectFirstAvailable() {
    auto items = GetDisplayedCosmetics();
    if (!items.empty()) {
        // If current selection is still in list, keep it
        bool found = false;
        for (const auto* it : items) {
            if (it && it->id == m_selectedCosmeticId) {
                found = true;
                break;
            }
        }
        if (!found) {
            SelectCosmetic(items.front()->id);
        }
    } else {
        m_selectedCosmeticId.clear();
        m_availableVariants.clear();
    }
}

} // namespace wildbound::cosmetics


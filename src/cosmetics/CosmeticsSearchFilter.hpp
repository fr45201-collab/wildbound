#pragma once

#include "CosmeticsRarity.hpp"
#include "CosmeticsOwnershipManager.hpp"
#include "../customization/CosmeticDefinition.hpp"
#include "../customization/CosmeticTypes.hpp"

#include <string>
#include <vector>
#include <optional>
#include <span>

namespace wildbound::cosmetics {

enum class OwnershipFilter : uint8_t {
    All = 0,
    OwnedOnly,
    LockedOnly
};

enum class EquipmentFilter : uint8_t {
    All = 0,
    CosmeticOnly,
    EquipmentOnly
};

enum class CosmeticSortOption : uint8_t {
    AlphabeticalAsc = 0,
    AlphabeticalDesc,
    RarityAsc,
    RarityDesc,
    OwnedFirst,
    LockedFirst
};

struct CosmeticFilterCriteria {
    std::string searchQuery;
    std::string category = "all"; // "all", "hair", "clothing", "armor", "headwear", "face", "backpacks", "footwear", "special"
    std::optional<customization::CustomizationSlot> slot;
    std::optional<CosmeticRarity> rarity;
    OwnershipFilter ownership = OwnershipFilter::All;
    EquipmentFilter equipment = EquipmentFilter::All;
    CosmeticSortOption sortOption = CosmeticSortOption::RarityDesc;

    void Reset() {
        searchQuery.clear();
        category = "all";
        slot.reset();
        rarity.reset();
        ownership = OwnershipFilter::All;
        equipment = EquipmentFilter::All;
        sortOption = CosmeticSortOption::RarityDesc;
    }
};

class CosmeticsSearchFilter {
public:
    static std::vector<const customization::CosmeticDefinition*> FilterAndSort(
        std::span<const customization::CosmeticDefinition* const> items,
        const CosmeticFilterCriteria& criteria,
        const CosmeticsOwnershipManager& ownershipMgr);

    static bool MatchesCriteria(
        const customization::CosmeticDefinition& item,
        const CosmeticFilterCriteria& criteria,
        const CosmeticsOwnershipManager& ownershipMgr);
};

} // namespace wildbound::cosmetics


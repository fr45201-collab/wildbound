#include "CosmeticsSearchFilter.hpp"
#include <algorithm>
#include <cctype>

namespace wildbound::cosmetics {

namespace {

std::string ToLower(std::string_view str) {
    std::string result;
    result.reserve(str.size());
    for (char c : str) {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return result;
}

bool MatchCategory(const customization::CosmeticDefinition& item, const std::string& cat) {
    if (cat.empty() || cat == "all" || cat == "All") return true;

    std::string lowerCat = ToLower(cat);
    std::string itemCat = ToLower(item.category);

    if (lowerCat == "featured") {
        CosmeticRarity r = StringToRarity(item.rarity);
        return r == CosmeticRarity::Legendary || r == CosmeticRarity::Mythic || r == CosmeticRarity::Epic;
    }
    if (lowerCat == "special") {
        CosmeticRarity r = StringToRarity(item.rarity);
        return r == CosmeticRarity::Legendary || r == CosmeticRarity::Mythic;
    }
    if (lowerCat == "hair" || lowerCat == "hairstyles") {
        return item.slot == customization::CustomizationSlot::Hair || itemCat.find("hair") != std::string::npos;
    }
    if (lowerCat == "clothing") {
        return (item.slot == customization::CustomizationSlot::Chest && !item.isGameplayEquipment) ||
               itemCat == "clothing" || itemCat == "costume" || itemCat == "tunic";
    }
    if (lowerCat == "armor") {
        return (item.slot == customization::CustomizationSlot::Chest && item.isGameplayEquipment) ||
               itemCat == "armor" || itemCat == "plate" || itemCat == "mail";
    }
    if (lowerCat == "headwear") {
        return item.slot == customization::CustomizationSlot::Headwear ||
               item.slot == customization::CustomizationSlot::Helmet ||
               itemCat.find("hat") != std::string::npos || itemCat.find("head") != std::string::npos ||
               itemCat.find("helm") != std::string::npos;
    }
    if (lowerCat == "face") {
        return item.slot == customization::CustomizationSlot::Face ||
               itemCat.find("face") != std::string::npos || itemCat.find("mask") != std::string::npos ||
               itemCat.find("goggle") != std::string::npos;
    }
    if (lowerCat == "backpacks" || lowerCat == "back") {
        return item.slot == customization::CustomizationSlot::Back ||
               itemCat.find("back") != std::string::npos;
    }
    if (lowerCat == "footwear" || lowerCat == "feet") {
        return item.slot == customization::CustomizationSlot::Feet ||
               itemCat.find("foot") != std::string::npos || itemCat.find("boot") != std::string::npos ||
               itemCat.find("shoe") != std::string::npos;
    }
    if (lowerCat == "legs") {
        return item.slot == customization::CustomizationSlot::Legs ||
               itemCat.find("leg") != std::string::npos || itemCat.find("pant") != std::string::npos;
    }
    if (lowerCat == "skin") {
        return item.slot == customization::CustomizationSlot::Skin;
    }

    return itemCat.find(lowerCat) != std::string::npos;
}

} // anonymous namespace

bool CosmeticsSearchFilter::MatchesCriteria(
    const customization::CosmeticDefinition& item,
    const CosmeticFilterCriteria& criteria,
    const CosmeticsOwnershipManager& ownershipMgr)
{
    // Category match
    if (!criteria.category.empty() && !MatchCategory(item, criteria.category)) {
        return false;
    }

    // Slot match
    if (criteria.slot.has_value() && item.slot != *criteria.slot) {
        return false;
    }

    // Rarity match
    if (criteria.rarity.has_value()) {
        CosmeticRarity itemRarity = StringToRarity(item.rarity);
        if (itemRarity != *criteria.rarity) {
            return false;
        }
    }

    // Ownership match
    if (criteria.ownership != OwnershipFilter::All) {
        bool isOwned = ownershipMgr.IsOwned(item.id);
        if (criteria.ownership == OwnershipFilter::OwnedOnly && !isOwned) {
            return false;
        }
        if (criteria.ownership == OwnershipFilter::LockedOnly && isOwned) {
            return false;
        }
    }

    // Equipment match
    if (criteria.equipment != EquipmentFilter::All) {
        if (criteria.equipment == EquipmentFilter::CosmeticOnly && item.isGameplayEquipment) {
            return false;
        }
        if (criteria.equipment == EquipmentFilter::EquipmentOnly && !item.isGameplayEquipment) {
            return false;
        }
    }

    // Search query match (case-insensitive substring check across name, desc, id, category, rarity)
    if (!criteria.searchQuery.empty()) {
        std::string query = ToLower(criteria.searchQuery);
        std::string lowerName = ToLower(item.displayName);
        std::string lowerDesc = ToLower(item.description);
        std::string lowerId = ToLower(item.id);
        std::string lowerCat = ToLower(item.category);
        std::string lowerRarity = ToLower(item.rarity);

        bool matched = (lowerName.find(query) != std::string::npos) ||
                       (lowerDesc.find(query) != std::string::npos) ||
                       (lowerId.find(query) != std::string::npos) ||
                       (lowerCat.find(query) != std::string::npos) ||
                       (lowerRarity.find(query) != std::string::npos);
        if (!matched) {
            return false;
        }
    }

    return true;
}

std::vector<const customization::CosmeticDefinition*> CosmeticsSearchFilter::FilterAndSort(
    std::span<const customization::CosmeticDefinition* const> items,
    const CosmeticFilterCriteria& criteria,
    const CosmeticsOwnershipManager& ownershipMgr)
{
    std::vector<const customization::CosmeticDefinition*> result;
    result.reserve(items.size());

    for (const auto* item : items) {
        if (item && MatchesCriteria(*item, criteria, ownershipMgr)) {
            result.push_back(item);
        }
    }

    // Sorting
    std::sort(result.begin(), result.end(), [&](const customization::CosmeticDefinition* a, const customization::CosmeticDefinition* b) {
        if (!a || !b) return a != nullptr;

        switch (criteria.sortOption) {
            case CosmeticSortOption::AlphabeticalAsc:
                return a->displayName < b->displayName;
            case CosmeticSortOption::AlphabeticalDesc:
                return a->displayName > b->displayName;
            case CosmeticSortOption::RarityAsc: {
                int rA = RarityToSortPriority(StringToRarity(a->rarity));
                int rB = RarityToSortPriority(StringToRarity(b->rarity));
                if (rA != rB) return rA < rB;
                return a->displayName < b->displayName;
            }
            case CosmeticSortOption::RarityDesc: {
                int rA = RarityToSortPriority(StringToRarity(a->rarity));
                int rB = RarityToSortPriority(StringToRarity(b->rarity));
                if (rA != rB) return rA > rB;
                return a->displayName < b->displayName;
            }
            case CosmeticSortOption::OwnedFirst: {
                bool ownedA = ownershipMgr.IsOwned(a->id);
                bool ownedB = ownershipMgr.IsOwned(b->id);
                if (ownedA != ownedB) return ownedA > ownedB;
                int rA = RarityToSortPriority(StringToRarity(a->rarity));
                int rB = RarityToSortPriority(StringToRarity(b->rarity));
                if (rA != rB) return rA > rB;
                return a->displayName < b->displayName;
            }
            case CosmeticSortOption::LockedFirst: {
                bool ownedA = ownershipMgr.IsOwned(a->id);
                bool ownedB = ownershipMgr.IsOwned(b->id);
                if (ownedA != ownedB) return ownedA < ownedB;
                int rA = RarityToSortPriority(StringToRarity(a->rarity));
                int rB = RarityToSortPriority(StringToRarity(b->rarity));
                if (rA != rB) return rA > rB;
                return a->displayName < b->displayName;
            }
            default:
                return a->displayName < b->displayName;
        }
    });

    return result;
}

} // namespace wildbound::cosmetics


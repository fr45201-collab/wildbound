#pragma once

#include <string>
#include <string_view>
#include <raylib.h>

namespace wildbound::cosmetics {

enum class CosmeticRarity {
    Common = 0,
    Uncommon = 1,
    Rare = 2,
    Epic = 3,
    Legendary = 4,
    Mythic = 5
};

struct RarityMetadata {
    CosmeticRarity rarity = CosmeticRarity::Common;
    std::string_view id = "common";
    std::string_view displayName = "Common";
    Color uiColor = Color{ 200, 200, 200, 255 };
    Color borderColor = Color{ 160, 160, 160, 255 };
    int sortPriority = 0;
};

inline const RarityMetadata& GetRarityMetadata(CosmeticRarity rarity) noexcept {
    static constexpr RarityMetadata metadataTable[] = {
        { CosmeticRarity::Common,    "common",    "Common",    Color{ 210, 210, 210, 255 }, Color{ 140, 140, 140, 255 }, 0 },
        { CosmeticRarity::Uncommon,  "uncommon",  "Uncommon",  Color{  70, 210, 100, 255 }, Color{  40, 160,  60, 255 }, 1 },
        { CosmeticRarity::Rare,      "rare",      "Rare",      Color{  60, 150, 250, 255 }, Color{  30, 100, 200, 255 }, 2 },
        { CosmeticRarity::Epic,      "epic",      "Epic",      Color{ 190,  80, 250, 255 }, Color{ 130,  40, 190, 255 }, 3 },
        { CosmeticRarity::Legendary, "legendary", "Legendary", Color{ 250, 180,  40, 255 }, Color{ 200, 120,  20, 255 }, 4 },
        { CosmeticRarity::Mythic,    "mythic",    "Mythic",    Color{ 250,  50,  80, 255 }, Color{ 180,  20,  40, 255 }, 5 }
    };
    size_t idx = static_cast<size_t>(rarity);
    if (idx < 6) return metadataTable[idx];
    return metadataTable[0];
}

inline std::string_view RarityToString(CosmeticRarity rarity) noexcept {
    return GetRarityMetadata(rarity).id;
}

inline std::string_view RarityToDisplayName(CosmeticRarity rarity) noexcept {
    return GetRarityMetadata(rarity).displayName;
}

inline Color RarityToColor(CosmeticRarity rarity) noexcept {
    return GetRarityMetadata(rarity).uiColor;
}

inline Color RarityToBorderColor(CosmeticRarity rarity) noexcept {
    return GetRarityMetadata(rarity).borderColor;
}

inline int RarityToSortPriority(CosmeticRarity rarity) noexcept {
    return GetRarityMetadata(rarity).sortPriority;
}

inline CosmeticRarity StringToRarity(std::string_view str) noexcept {
    if (str == "mythic" || str == "Mythic") return CosmeticRarity::Mythic;
    if (str == "legendary" || str == "Legendary") return CosmeticRarity::Legendary;
    if (str == "epic" || str == "Epic") return CosmeticRarity::Epic;
    if (str == "rare" || str == "Rare") return CosmeticRarity::Rare;
    if (str == "uncommon" || str == "Uncommon") return CosmeticRarity::Uncommon;
    return CosmeticRarity::Common;
}

} // namespace wildbound::cosmetics


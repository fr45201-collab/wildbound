#pragma once

#include <string>
#include <vector>
#include <raylib.h>
#include <nlohmann/json.hpp>

namespace wildbound::cosmetics {

struct CosmeticColorVariant {
    std::string id;
    std::string displayName;
    Color tint = WHITE;
    std::string hexColor = "#FFFFFF";
    std::string assetKeyOverride = "";

    [[nodiscard]] bool operator==(const CosmeticColorVariant& other) const noexcept {
        return id == other.id;
    }

    nlohmann::json ToJson() const {
        return nlohmann::json{
            { "id", id },
            { "displayName", displayName },
            { "tint", { tint.r, tint.g, tint.b, tint.a } },
            { "hexColor", hexColor },
            { "assetKeyOverride", assetKeyOverride }
        };
    }

    static CosmeticColorVariant FromJson(const nlohmann::json& j) {
        CosmeticColorVariant v;
        if (j.contains("id") && j["id"].is_string()) v.id = j["id"].get<std::string>();
        if (j.contains("displayName") && j["displayName"].is_string()) v.displayName = j["displayName"].get<std::string>();
        if (j.contains("hexColor") && j["hexColor"].is_string()) v.hexColor = j["hexColor"].get<std::string>();
        if (j.contains("assetKeyOverride") && j["assetKeyOverride"].is_string()) v.assetKeyOverride = j["assetKeyOverride"].get<std::string>();
        if (j.contains("tint") && j["tint"].is_array() && j["tint"].size() >= 4) {
            v.tint = Color{
                static_cast<unsigned char>(j["tint"][0].get<int>()),
                static_cast<unsigned char>(j["tint"][1].get<int>()),
                static_cast<unsigned char>(j["tint"][2].get<int>()),
                static_cast<unsigned char>(j["tint"][3].get<int>())
            };
        }
        return v;
    }
};

/**
 * Common color variation palettes for cosmetic categories.
 */
class ColorVariantPalette {
public:
    static std::vector<CosmeticColorVariant> GetDefaultVariants() {
        return {
            { "variant_default", "Default", WHITE, "#FFFFFF", "" }
        };
    }

    static std::vector<CosmeticColorVariant> GetHairVariants() {
        return {
            { "hair_blonde",   "Blonde",     Color{ 245, 220, 130, 255 }, "#F5DC82", "" },
            { "hair_brown",    "Chestnut",   Color{ 110,  70,  40, 255 }, "#6E4628", "" },
            { "hair_black",    "Raven",      Color{  35,  35,  40, 255 }, "#232328", "" },
            { "hair_ginger",   "Auburn",     Color{ 190,  85,  35, 255 }, "#BE5523", "" },
            { "hair_silver",   "Silver",     Color{ 210, 215, 225, 255 }, "#D2D7E1", "" },
            { "hair_crimson",  "Crimson",    Color{ 180,  30,  50, 255 }, "#B41E32", "" },
            { "hair_azure",    "Azure",      Color{  50, 140, 220, 255 }, "#328CDC", "" },
            { "hair_emerald",  "Emerald",    Color{  40, 160,  90, 255 }, "#28A05A", "" },
            { "hair_violet",   "Amethyst",   Color{ 140,  60, 190, 255 }, "#8C3CBE", "" }
        };
    }

    static std::vector<CosmeticColorVariant> GetClothVariants() {
        return {
            { "cloth_classic", "Classic",    WHITE,                       "#FFFFFF", "" },
            { "cloth_crimson", "Crimson",    Color{ 210,  60,  60, 255 }, "#D23C3C", "" },
            { "cloth_navy",    "Navy",       Color{  50,  80, 160, 255 }, "#3250A0", "" },
            { "cloth_forest",  "Forest",     Color{  50, 130,  70, 255 }, "#328246", "" },
            { "cloth_shadow",  "Midnight",   Color{  50,  50,  60, 255 }, "#32323C", "" },
            { "cloth_royal",   "Royal Gold", Color{ 230, 190,  60, 255 }, "#E6BE3C", "" },
            { "cloth_purple",  "Regal",      Color{ 130,  50, 150, 255 }, "#823296", "" },
            { "cloth_snow",    "Ivory",      Color{ 240, 240, 245, 255 }, "#F0F0F5", "" }
        };
    }

    static std::vector<CosmeticColorVariant> GetMetalVariants() {
        return {
            { "metal_iron",    "Iron",       WHITE,                       "#FFFFFF", "" },
            { "metal_steel",   "Steel",      Color{ 180, 190, 205, 255 }, "#B4BED7", "" },
            { "metal_gold",    "Gilded",     Color{ 240, 200,  70, 255 }, "#F0C846", "" },
            { "metal_dark",    "Darksteel",  Color{  70,  75,  85, 255 }, "#464B55", "" },
            { "metal_copper",  "Bronze",     Color{ 200, 120,  80, 255 }, "#C87850", "" },
            { "metal_mithril", "Mithril",    Color{ 120, 210, 240, 255 }, "#78D2F0", "" }
        };
    }

    static std::vector<CosmeticColorVariant> GetVariantsForCategory(const std::string& category) {
        if (category == "hair" || category == "hairstyle") {
            return GetHairVariants();
        }
        if (category == "armor" || category == "helmet" || category == "shield") {
            return GetMetalVariants();
        }
        if (category == "clothing" || category == "shirt" || category == "tunic" || 
            category == "pants" || category == "boots" || category == "headwear" ||
            category == "face" || category == "back") {
            return GetClothVariants();
        }
        return GetDefaultVariants();
    }
};

} // namespace wildbound::cosmetics


#include "ItemManager.hpp"
#include "../resources/AssetManager.hpp"
#include "../core/Logger.hpp"

#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using wildbound::core::Logger;

namespace wildbound::items {

ItemManager::ItemManager() = default;
ItemManager::~ItemManager() = default;

bool ItemManager::Initialize(const std::string& catalogPath) {
    m_items.clear();
    m_itemIndexMap.clear();

    Logger::Info("ITEM", "Initializing ItemManager from catalog: " + catalogPath);

    std::filesystem::path catPath(catalogPath);
    if (!std::filesystem::exists(catPath)) {
        Logger::Error("ITEM", "Item catalog file does not exist: " + catalogPath);
        return false;
    }

    std::ifstream file(catPath);
    if (!file.is_open()) {
        Logger::Error("ITEM", "Failed to open item catalog file: " + catalogPath);
        return false;
    }

    try {
        nlohmann::json root;
        file >> root;

        if (!root.contains("items") || !root["items"].is_array()) {
            Logger::Error("ITEM", "Invalid item catalog: missing 'items' array");
            return false;
        }

        for (const auto& iJson : root["items"]) {
            ItemConfig cfg;
            cfg.id = iJson.value("id", "");
            if (cfg.id.empty()) {
                Logger::Warn("ITEM", "Skipping item with empty ID.");
                continue;
            }

            cfg.displayName = iJson.value("display_name", cfg.id);
            std::string catStr = iJson.value("category", "collectible");
            cfg.category = StringToItemCategory(catStr);
            cfg.subcategory = iJson.value("subcategory", "");
            cfg.description = iJson.value("description", "");

            // Visual Assets (Follow AMS naming convention: tex_<id>_icon, tex_<id>_world)
            cfg.visuals.iconTextureId = "tex_" + cfg.id + "_icon";
            cfg.visuals.worldTextureId = "tex_" + cfg.id + "_world";

            // Biome tags
            if (iJson.contains("biome_tags") && iJson["biome_tags"].is_array()) {
                cfg.biomeTags.clear();
                for (const auto& bt : iJson["biome_tags"]) {
                    cfg.biomeTags.push_back(bt.get<std::string>());
                }
            }

            cfg.rarityTier = iJson.value("rarity_tier", "common");

            // Stackability and max stack sizes based on category
            if (cfg.category == ItemCategory::Consumable) {
                cfg.stackable = true;
                cfg.maxStackSize = 10;
                cfg.weight = 0.5f;

                // Consumable effect values
                if (cfg.id == "con_health_potion_small_001" || cfg.id == "pot_health_001") {
                    cfg.consumable.healAmount = 50.0f;
                    cfg.consumable.effectType = "heal";
                } else if (cfg.id == "con_health_potion_large_001") {
                    cfg.consumable.healAmount = 100.0f;
                    cfg.consumable.effectType = "heal";
                } else if (cfg.id == "con_bread_001" || cfg.id == "food_bread_001") {
                    cfg.consumable.healAmount = 15.0f;
                    cfg.consumable.effectType = "heal";
                } else if (cfg.id == "con_apple_001" || cfg.id == "food_apple_001") {
                    cfg.consumable.healAmount = 20.0f;
                    cfg.consumable.effectType = "heal";
                } else if (cfg.id == "con_cooked_meat_001" || cfg.id == "food_meat_cooked_001") {
                    cfg.consumable.healAmount = 35.0f;
                    cfg.consumable.effectType = "heal";
                } else if (cfg.id == "con_healing_herb_001" || cfg.id == "food_berries_001") {
                    cfg.consumable.healAmount = 25.0f;
                    cfg.consumable.effectType = "heal";
                } else if (cfg.id == "con_stamina_potion_001" || cfg.id == "pot_stamina_001") {
                    cfg.consumable.staminaAmount = 50.0f;
                    cfg.consumable.effectType = "stamina";
                } else if (cfg.id == "con_mana_potion_001") {
                    cfg.consumable.effectType = "mana";
                } else if (cfg.id == "con_antidote_001" || cfg.id == "pot_antidote_001") {
                    cfg.consumable.effectType = "antidote";
                }
            } else if (cfg.category == ItemCategory::SpecialItem) {
                cfg.stackable = false;
                cfg.maxStackSize = 1;
                cfg.weight = 1.0f;
            } else if (cfg.category == ItemCategory::Weapon || cfg.category == ItemCategory::Armor) {
                cfg.stackable = false;
                cfg.maxStackSize = 1;
                cfg.weight = 3.0f;
            } else {
                // Collectibles, materials, resources
                cfg.stackable = true;
                cfg.maxStackSize = 99;
                cfg.weight = 0.1f;
            }

            if (iJson.contains("world_dimensions") && iJson["world_dimensions"].is_object()) {
                cfg.worldDimensions.x = iJson["world_dimensions"].value("x", 32.0f);
                cfg.worldDimensions.y = iJson["world_dimensions"].value("y", 32.0f);
            }

            size_t idx = m_items.size();
            m_itemIndexMap[cfg.id] = idx;
            m_items.push_back(std::move(cfg));
        }

        Logger::Info("ITEM", "Successfully loaded " + std::to_string(m_items.size()) +
                     " item definitions from catalog.");
        return !m_items.empty();
    } catch (const std::exception& e) {
        Logger::Error("ITEM", "Error parsing item catalog: " + std::string(e.what()));
        return false;
    }
}

bool ItemManager::RegisterDefaultAmmunition() {
    ItemConfig arrow;
    arrow.id = "arrow_iron_001";
    arrow.displayName = "Iron Arrow";
    arrow.category = ItemCategory::Ammunition;
    arrow.subcategory = "arrow";
    arrow.description = "A sturdy iron-tipped arrow designed for bows.";
    arrow.visuals.iconTextureId = "tex_arrow_iron_001_icon";
    arrow.visuals.worldTextureId = "tex_arrow_iron_001_ground";
    arrow.stackable = true;
    arrow.maxStackSize = 30;
    arrow.weight = 0.05f;
    arrow.rarityTier = "common";
    arrow.ammunition.compatibleWeaponType = "bow";
    arrow.ammunition.bonusDamage = 5.0f;
    return RegisterItem(arrow);
}

bool ItemManager::RegisterItem(ItemConfig item) {
    if (item.id.empty()) return false;
    auto it = m_itemIndexMap.find(item.id);
    if (it != m_itemIndexMap.end()) {
        m_items[it->second] = std::move(item);
        return true;
    }
    size_t idx = m_items.size();
    m_itemIndexMap[item.id] = idx;
    m_items.push_back(std::move(item));
    return true;
}

const ItemConfig* ItemManager::GetItem(const std::string& id) const {
    auto it = m_itemIndexMap.find(id);
    if (it != m_itemIndexMap.end() && it->second < m_items.size()) {
        return &m_items[it->second];
    }
    return nullptr;
}

bool ItemManager::HasItem(const std::string& id) const {
    return m_itemIndexMap.find(id) != m_itemIndexMap.end();
}

std::vector<const ItemConfig*> ItemManager::GetItemsByCategory(ItemCategory category) const {
    std::vector<const ItemConfig*> result;
    for (const auto& item : m_items) {
        if (item.category == category) {
            result.push_back(&item);
        }
    }
    return result;
}

std::vector<const ItemConfig*> ItemManager::GetItemsByBiome(const std::string& biomeTag) const {
    std::vector<const ItemConfig*> result;
    for (const auto& item : m_items) {
        if (item.HasBiomeTag(biomeTag)) {
            result.push_back(&item);
        }
    }
    return result;
}

bool ItemManager::ValidateAssets(const resources::AssetManager& assetMgr, std::vector<std::string>& outErrors) const {
    outErrors.clear();
    const auto& reg = assetMgr.GetRegistry();

    for (const auto& item : m_items) {
        // Check icon texture
        if (!reg.HasAsset(item.visuals.iconTextureId)) {
            outErrors.push_back("[" + item.id + "] Missing icon texture in AMS: " + item.visuals.iconTextureId);
        }

        // Check world texture
        if (!reg.HasAsset(item.visuals.worldTextureId)) {
            outErrors.push_back("[" + item.id + "] Missing world texture in AMS: " + item.visuals.worldTextureId);
        }
    }

    if (!outErrors.empty()) {
        Logger::Warn("ITEM", "Item asset validation found " + std::to_string(outErrors.size()) + " errors.");
        return false;
    }

    Logger::Info("ITEM", "Item asset validation passed 100% for all " +
                 std::to_string(m_items.size()) + " items.");
    return true;
}

} // namespace wildbound::items


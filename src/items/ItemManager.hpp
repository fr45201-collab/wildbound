#pragma once

#include "ItemData.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::items {

/**
 * High-level manager for the Wildbound Item & Collectible Asset Collection.
 * Loads item definitions, provides category and ID queries, and validates AMS asset resolution.
 */
class ItemManager {
public:
    ItemManager();
    ~ItemManager();

    bool Initialize(const std::string& catalogPath = "data/items/item_catalog.json");

    [[nodiscard]] const ItemConfig* GetItem(const std::string& id) const;
    [[nodiscard]] bool HasItem(const std::string& id) const;
    bool RegisterItem(ItemConfig item);
    bool RegisterDefaultAmmunition();
    [[nodiscard]] const std::vector<ItemConfig>& GetAllItems() const { return m_items; }
    [[nodiscard]] std::vector<const ItemConfig*> GetItemsByCategory(ItemCategory category) const;
    [[nodiscard]] std::vector<const ItemConfig*> GetItemsByBiome(const std::string& biomeTag) const;
    [[nodiscard]] size_t GetItemCount() const { return m_items.size(); }

    // Validation against AMS registry
    [[nodiscard]] bool ValidateAssets(const resources::AssetManager& assetMgr, std::vector<std::string>& outErrors) const;

private:
    std::vector<ItemConfig> m_items;
    std::unordered_map<std::string, size_t> m_itemIndexMap;
};

} // namespace wildbound::items


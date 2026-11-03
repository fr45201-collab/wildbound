#pragma once

#include "AssetMetadata.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

namespace wildbound::resources {

/**
 * Thread-safe centralized catalog and query database of all registered engine assets.
 */
class AssetRegistry {
public:
    AssetRegistry();
    ~AssetRegistry();

    bool RegisterAsset(const AssetMetadata& metadata);
    bool UnregisterAsset(const std::string& id);
    void Clear();

    bool HasAsset(const std::string& id) const;
    const AssetMetadata* GetMetadata(const std::string& id) const;

    std::vector<AssetMetadata> GetAll() const;
    std::vector<AssetMetadata> GetByCategory(const std::string& category) const;
    std::vector<AssetMetadata> GetByType(AssetType type) const;
    std::vector<std::string> GetCategories() const;

    bool LoadManifest(const std::string& manifestPath);
    bool SaveManifest(const std::string& manifestPath) const;

    size_t GetAssetCount() const;

private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, AssetMetadata> m_assets;
    std::unordered_map<std::string, std::string> m_fileToIdMap;
};

} // namespace wildbound::resources


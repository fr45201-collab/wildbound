#include "AssetRegistry.hpp"
#include "../core/Logger.hpp"
#include <fstream>
#include <set>
#include <filesystem>

using wildbound::core::Logger;
namespace fs = std::filesystem;

namespace wildbound::resources {

AssetRegistry::AssetRegistry() = default;

AssetRegistry::~AssetRegistry() {
    Clear();
}

bool AssetRegistry::RegisterAsset(const AssetMetadata& metadata) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (metadata.id.empty()) {
        Logger::Error("ASSET_REGISTRY", "Cannot register asset with empty identifier.");
        return false;
    }
    if (metadata.file.empty()) {
        Logger::Error("ASSET_REGISTRY", "Cannot register asset '" + metadata.id + "' with empty file path.");
        return false;
    }

    // Check duplicate ID
    if (m_assets.find(metadata.id) != m_assets.end()) {
        Logger::Warn("ASSET_REGISTRY", "Duplicate asset identifier detected: '" + metadata.id + "'. Overwriting.");
    }

    // Check duplicate file mapping
    auto fileIt = m_fileToIdMap.find(metadata.file);
    if (fileIt != m_fileToIdMap.end() && fileIt->second != metadata.id) {
        Logger::Warn("ASSET_REGISTRY", "File '" + metadata.file + "' already registered under ID '" +
                     fileIt->second + "'. Cross-referencing.");
    }

    m_assets[metadata.id] = metadata;
    m_fileToIdMap[metadata.file] = metadata.id;

    Logger::Debug("ASSET_REGISTRY", "Registered asset: [" + metadata.id + "] (" + metadata.file + ")");
    return true;
}

bool AssetRegistry::UnregisterAsset(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_assets.find(id);
    if (it != m_assets.end()) {
        m_fileToIdMap.erase(it->second.file);
        m_assets.erase(it);
        return true;
    }
    return false;
}

void AssetRegistry::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_assets.clear();
    m_fileToIdMap.clear();
}

bool AssetRegistry::HasAsset(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_assets.find(id) != m_assets.end()) return true;
    return m_fileToIdMap.find(id) != m_fileToIdMap.end();
}

const AssetMetadata* AssetRegistry::GetMetadata(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_assets.find(id);
    if (it != m_assets.end()) {
        return &it->second;
    }
    auto fileIt = m_fileToIdMap.find(id);
    if (fileIt != m_fileToIdMap.end()) {
        auto it2 = m_assets.find(fileIt->second);
        if (it2 != m_assets.end()) {
            return &it2->second;
        }
    }
    return nullptr;
}

std::vector<AssetMetadata> AssetRegistry::GetAll() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<AssetMetadata> result;
    result.reserve(m_assets.size());
    for (const auto& [id, meta] : m_assets) {
        result.push_back(meta);
    }
    return result;
}

std::vector<AssetMetadata> AssetRegistry::GetByCategory(const std::string& category) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<AssetMetadata> result;
    for (const auto& [id, meta] : m_assets) {
        if (meta.category == category) {
            result.push_back(meta);
        }
    }
    return result;
}

std::vector<AssetMetadata> AssetRegistry::GetByType(AssetType type) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<AssetMetadata> result;
    for (const auto& [id, meta] : m_assets) {
        if (meta.type == type) {
            result.push_back(meta);
        }
    }
    return result;
}

std::vector<std::string> AssetRegistry::GetCategories() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::set<std::string> cats;
    for (const auto& [id, meta] : m_assets) {
        if (!meta.category.empty()) {
            cats.insert(meta.category);
        }
    }
    return std::vector<std::string>(cats.begin(), cats.end());
}

size_t AssetRegistry::GetAssetCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_assets.size();
}

bool AssetRegistry::LoadManifest(const std::string& manifestPath) {
    if (!fs::exists(manifestPath)) {
        Logger::Error("ASSET_REGISTRY", "Manifest file not found: " + manifestPath);
        return false;
    }

    try {
        std::ifstream file(manifestPath);
        if (!file.is_open()) {
            Logger::Error("ASSET_REGISTRY", "Unable to open manifest file: " + manifestPath);
            return false;
        }

        nlohmann::json root;
        file >> root;

        if (!root.contains("assets") || !root["assets"].is_array()) {
            Logger::Error("ASSET_REGISTRY", "Manifest JSON missing root 'assets' array.");
            return false;
        }

        size_t count = 0;
        for (const auto& item : root["assets"]) {
            AssetMetadata meta = AssetMetadata::FromJson(item);
            if (RegisterAsset(meta)) {
                count++;
            }
        }

        Logger::Info("ASSET_REGISTRY", "Loaded manifest (" + manifestPath + ") with " +
                     std::to_string(count) + " assets registered.");
        return true;
    }
    catch (const std::exception& ex) {
        Logger::Error("ASSET_REGISTRY", "Manifest parsing exception: " + std::string(ex.what()));
        return false;
    }
}

bool AssetRegistry::SaveManifest(const std::string& manifestPath) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    try {
        nlohmann::json root;
        root["version"] = "1.0.0";
        root["assets"] = nlohmann::json::array();

        for (const auto& [id, meta] : m_assets) {
            root["assets"].push_back(meta.ToJson());
        }

        std::ofstream file(manifestPath);
        if (!file.is_open()) {
            Logger::Error("ASSET_REGISTRY", "Failed to open manifest for writing: " + manifestPath);
            return false;
        }

        file << root.dump(4);
        return true;
    }
    catch (const std::exception& ex) {
        Logger::Error("ASSET_REGISTRY", "Failed to serialize manifest: " + std::string(ex.what()));
        return false;
    }
}

} // namespace wildbound::resources


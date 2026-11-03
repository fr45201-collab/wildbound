#pragma once

#include <string>
#include <string_view>
#include <nlohmann/json.hpp>

namespace wildbound::resources {

enum class AssetType {
    Unknown = 0,
    Texture,
    SpriteSheet,
    Font,
    Sound,
    Music,
    Data
};

enum class AssetStatus {
    Approved = 0,
    LicenseReviewRequired,
    Missing,
    Deprecated
};

struct AssetMetadata {
    std::string id;
    std::string file;
    AssetType type = AssetType::Unknown;
    std::string category;
    std::string source;
    std::string author;
    std::string license;
    bool commercialUse = true;
    bool attributionRequired = false;
    std::string checksum;
    int width = 0;
    int height = 0;
    AssetStatus status = AssetStatus::LicenseReviewRequired;

    // Helper utilities
    bool IsValid() const { return !id.empty() && !file.empty(); }
    bool IsApproved() const { return status == AssetStatus::Approved; }

    static std::string_view TypeToString(AssetType t);
    static AssetType StringToType(std::string_view s);

    static std::string_view StatusToString(AssetStatus s);
    static AssetStatus StringToStatus(std::string_view s);

    nlohmann::json ToJson() const;
    static AssetMetadata FromJson(const nlohmann::json& j);
};

} // namespace wildbound::resources


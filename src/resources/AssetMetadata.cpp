#include "AssetMetadata.hpp"
#include "../core/Logger.hpp"

using wildbound::core::Logger;

namespace wildbound::resources {

std::string_view AssetMetadata::TypeToString(AssetType t) {
    switch (t) {
        case AssetType::Texture:     return "texture";
        case AssetType::SpriteSheet: return "spritesheet";
        case AssetType::Font:        return "font";
        case AssetType::Sound:       return "sound";
        case AssetType::Music:       return "music";
        case AssetType::Data:        return "data";
        default:                     return "unknown";
    }
}

AssetType AssetMetadata::StringToType(std::string_view s) {
    if (s == "texture")     return AssetType::Texture;
    if (s == "spritesheet") return AssetType::SpriteSheet;
    if (s == "font")        return AssetType::Font;
    if (s == "sound")       return AssetType::Sound;
    if (s == "audio")       return AssetType::Sound;
    if (s == "music")       return AssetType::Music;
    if (s == "data")        return AssetType::Data;
    return AssetType::Unknown;
}

std::string_view AssetMetadata::StatusToString(AssetStatus s) {
    switch (s) {
        case AssetStatus::Approved:              return "approved";
        case AssetStatus::LicenseReviewRequired: return "license_review_required";
        case AssetStatus::Missing:                return "missing";
        case AssetStatus::Deprecated:             return "deprecated";
        default:                                 return "unknown";
    }
}

AssetStatus AssetMetadata::StringToStatus(std::string_view s) {
    if (s == "approved" || s == "Approved")                return AssetStatus::Approved;
    if (s == "license_review_required" || s == "LicenseReviewRequired") return AssetStatus::LicenseReviewRequired;
    if (s == "missing" || s == "Missing")                 return AssetStatus::Missing;
    if (s == "deprecated" || s == "Deprecated")              return AssetStatus::Deprecated;
    return AssetStatus::LicenseReviewRequired;
}

nlohmann::json AssetMetadata::ToJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["file"] = file;
    j["type"] = TypeToString(type);
    j["category"] = category;
    j["source"] = source;
    j["author"] = author;
    j["license"] = license;
    j["commercial_use"] = commercialUse;
    j["attribution_required"] = attributionRequired;
    j["checksum"] = checksum;
    if (type == AssetType::Sound || type == AssetType::Music || duration > 0.0f) {
        j["duration"] = duration;
    }
    j["width"] = width;
    j["height"] = height;
    j["status"] = StatusToString(status);
    return j;
}

AssetMetadata AssetMetadata::FromJson(const nlohmann::json& j) {
    AssetMetadata meta;
    meta.id = j.value("id", "");

    // Path resolution: canonical "file" preferred, legacy "path" as fallback
    std::string fileKey = j.value("file", "");
    std::string pathKey = j.value("path", "");
    if (!fileKey.empty() && !pathKey.empty() && fileKey != pathKey) {
        Logger::Warn("ASSET_METADATA", "Asset '" + meta.id + "' specifies conflicting 'file' and 'path'. Using canonical 'file'.");
        meta.file = fileKey;
    } else if (!fileKey.empty()) {
        meta.file = fileKey;
    } else if (!pathKey.empty()) {
        Logger::Warn("ASSET_METADATA", "Asset '" + meta.id + "' uses legacy key 'path'. Mapped to canonical 'file'.");
        meta.file = pathKey;
    }

    meta.type = StringToType(j.value("type", "unknown"));
    meta.category = j.value("category", "");
    meta.source = j.value("source", "");
    meta.author = j.value("author", "");
    meta.license = j.value("license", "");

    // Commercial use & attribution: support both snake_case and camelCase
    if (j.contains("commercial_use")) {
        meta.commercialUse = j["commercial_use"].get<bool>();
    } else if (j.contains("commercialUse")) {
        Logger::Warn("ASSET_METADATA", "Asset '" + meta.id + "' uses camelCase 'commercialUse'. Normalizing.");
        meta.commercialUse = j["commercialUse"].get<bool>();
    } else {
        meta.commercialUse = true;
    }

    if (j.contains("attribution_required")) {
        meta.attributionRequired = j["attribution_required"].get<bool>();
    } else if (j.contains("attributionRequired")) {
        Logger::Warn("ASSET_METADATA", "Asset '" + meta.id + "' uses camelCase 'attributionRequired'. Normalizing.");
        meta.attributionRequired = j["attributionRequired"].get<bool>();
    } else {
        meta.attributionRequired = false;
    }

    meta.checksum = j.value("checksum", "");
    meta.duration = j.value("duration", 0.0f);

    // Dimensions: check root width/height, fallback to nested metadata.width/height
    meta.width = j.value("width", 0);
    meta.height = j.value("height", 0);
    if (meta.width == 0 && meta.height == 0 && j.contains("metadata") && j["metadata"].is_object()) {
        meta.width = j["metadata"].value("width", 0);
        meta.height = j["metadata"].value("height", 0);
        if (meta.width != 0 || meta.height != 0) {
            Logger::Warn("ASSET_METADATA", "Asset '" + meta.id + "' dimensions extracted from nested 'metadata' object.");
        }
    }

    meta.status = StringToStatus(j.value("status", "license_review_required"));
    return meta;
}

} // namespace wildbound::resources


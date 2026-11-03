#include "AssetMetadata.hpp"

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
    if (s == "approved")                return AssetStatus::Approved;
    if (s == "license_review_required") return AssetStatus::LicenseReviewRequired;
    if (s == "missing")                 return AssetStatus::Missing;
    if (s == "deprecated")              return AssetStatus::Deprecated;
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
    j["width"] = width;
    j["height"] = height;
    j["status"] = StatusToString(status);
    return j;
}

AssetMetadata AssetMetadata::FromJson(const nlohmann::json& j) {
    AssetMetadata meta;
    meta.id = j.value("id", "");
    meta.file = j.value("file", "");
    meta.type = StringToType(j.value("type", "unknown"));
    meta.category = j.value("category", "");
    meta.source = j.value("source", "");
    meta.author = j.value("author", "");
    meta.license = j.value("license", "");
    meta.commercialUse = j.value("commercial_use", true);
    meta.attributionRequired = j.value("attribution_required", false);
    meta.checksum = j.value("checksum", "");
    meta.width = j.value("width", 0);
    meta.height = j.value("height", 0);
    meta.status = StringToStatus(j.value("status", "license_review_required"));
    return meta;
}

} // namespace wildbound::resources


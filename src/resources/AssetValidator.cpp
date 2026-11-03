#include "AssetValidator.hpp"
#include "../core/Logger.hpp"
#include <filesystem>
#include <algorithm>

using wildbound::core::Logger;
namespace fs = std::filesystem;

namespace wildbound::resources {

bool AssetValidator::IsExtensionSupported(const std::string& ext, AssetType type) {
    std::string lowerExt = ext;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    switch (type) {
        case AssetType::Texture:
        case AssetType::SpriteSheet:
            return (lowerExt == ".png" || lowerExt == ".jpg" || lowerExt == ".jpeg" || lowerExt == ".bmp");
        case AssetType::Font:
            return (lowerExt == ".ttf" || lowerExt == ".otf");
        case AssetType::Sound:
        case AssetType::Music:
            return (lowerExt == ".wav" || lowerExt == ".ogg" || lowerExt == ".mp3");
        case AssetType::Data:
            return (lowerExt == ".json" || lowerExt == ".lua" || lowerExt == ".xml");
        default:
            return true;
    }
}

bool AssetValidator::ValidateAssetFile(const AssetMetadata& meta, const std::string& assetRoot, ValidationReport& report) {
    bool hasError = false;

    // 1. Check ID validity
    if (meta.id.empty()) {
        report.errorsCount++;
        report.issues.push_back({ ValidationSeverity::Error, "", "Asset entry has an empty ID." });
        return false;
    }

    // 2. Check File Path validity
    if (meta.file.empty()) {
        report.errorsCount++;
        report.issues.push_back({ ValidationSeverity::Error, meta.id, "Asset file path is empty." });
        return false;
    }

    // 3. Resolve path on disk
    std::string candidate1 = assetRoot + "/" + meta.file;
    std::string candidate2 = meta.file;
    std::string resolvedPath = fs::exists(candidate1) ? candidate1 : (fs::exists(candidate2) ? candidate2 : "");

    if (resolvedPath.empty()) {
        report.errorsCount++;
        report.issues.push_back({
            ValidationSeverity::Error,
            meta.id,
            "Asset file not found on disk: " + meta.file
        });
        hasError = true;
    } else {
        // Check format extension
        fs::path p(resolvedPath);
        std::string ext = p.extension().string();
        if (!IsExtensionSupported(ext, meta.type)) {
            report.warningsCount++;
            report.issues.push_back({
                ValidationSeverity::Warning,
                meta.id,
                "File extension '" + ext + "' is not standard for asset type."
            });
        }
    }

    // 4. Validate Licensing Status
    if (meta.status == AssetStatus::LicenseReviewRequired) {
        report.warningsCount++;
        report.issues.push_back({
            ValidationSeverity::Warning,
            meta.id,
            "Asset license status requires review: '" + meta.file + "' (source: " +
            (meta.source.empty() ? "unknown" : meta.source) + ")"
        });
    }

    if (!hasError) {
        report.passedCount++;
        return true;
    }
    return false;
}

ValidationReport AssetValidator::ValidateAll(const AssetRegistry& registry, const std::string& assetRoot) {
    ValidationReport report;
    auto assets = registry.GetAll();
    report.totalAssetsChecked = assets.size();

    for (const auto& meta : assets) {
        ValidateAssetFile(meta, assetRoot, report);
    }

    Logger::Info("ASSET_VALIDATOR", "Validation complete: " +
                 std::to_string(report.passedCount) + " passed, " +
                 std::to_string(report.warningsCount) + " warnings, " +
                 std::to_string(report.errorsCount) + " errors.");

    return report;
}

} // namespace wildbound::resources


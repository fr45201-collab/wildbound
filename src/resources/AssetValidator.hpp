#pragma once

#include "AssetRegistry.hpp"
#include <string>
#include <vector>

namespace wildbound::resources {

enum class ValidationSeverity {
    Info,
    Warning,
    Error
};

struct ValidationIssue {
    ValidationSeverity severity;
    std::string assetId;
    std::string message;
};

struct ValidationReport {
    size_t totalAssetsChecked = 0;
    size_t passedCount = 0;
    size_t warningsCount = 0;
    size_t errorsCount = 0;
    std::vector<ValidationIssue> issues;

    bool IsClean() const { return errorsCount == 0 && warningsCount == 0; }
    bool HasErrors() const { return errorsCount > 0; }
};

/**
 * Validates manifest records, disk asset availability, format compliance,
 * and licensing status.
 */
class AssetValidator {
public:
    static ValidationReport ValidateAll(const AssetRegistry& registry, const std::string& assetRoot = "assets");
    static bool ValidateAssetFile(const AssetMetadata& meta, const std::string& assetRoot, ValidationReport& report);
    static bool IsExtensionSupported(const std::string& ext, AssetType type);
};

} // namespace wildbound::resources


#pragma once

#include "Tilemap.hpp"
#include <string>
#include <vector>

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::world {

enum class MapIssueSeverity {
    Info,
    Warning,
    Error
};

struct MapValidationIssue {
    MapIssueSeverity severity;
    std::string element;
    std::string message;
};

struct MapValidationReport {
    std::string mapName;
    size_t errorsCount = 0;
    size_t warningsCount = 0;
    std::vector<MapValidationIssue> issues;

    bool IsValid() const { return errorsCount == 0; }
};

/**
 * Diagnostic auditing tool for tilemap layers, tileset links, and object metadata.
 */
class MapValidator {
public:
    static MapValidationReport Validate(const Tilemap& tilemap, resources::AssetManager* assetMgr = nullptr);
};

} // namespace wildbound::world


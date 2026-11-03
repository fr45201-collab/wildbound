#include "MapValidator.hpp"
#include "../resources/AssetManager.hpp"
#include "../core/Logger.hpp"

using wildbound::core::Logger;

namespace wildbound::world {

MapValidationReport MapValidator::Validate(const Tilemap& tilemap, resources::AssetManager* assetMgr) {
    MapValidationReport report;
    report.mapName = tilemap.GetName();

    // 1. Validate Map Dimensions
    if (tilemap.GetWidth() <= 0 || tilemap.GetHeight() <= 0) {
        report.errorsCount++;
        report.issues.push_back({ MapIssueSeverity::Error, "Dimensions", "Map width and height must be positive." });
    }
    if (tilemap.GetTileWidth() <= 0 || tilemap.GetTileHeight() <= 0) {
        report.errorsCount++;
        report.issues.push_back({ MapIssueSeverity::Error, "TileSize", "Tile width and height must be positive." });
    }

    // 2. Validate Tilesets
    if (tilemap.GetTilesets().empty()) {
        report.errorsCount++;
        report.issues.push_back({ MapIssueSeverity::Error, "Tilesets", "Map contains zero tilesets." });
    } else {
        for (const auto& ts : tilemap.GetTilesets()) {
            if (ts.GetTileCount() <= 0) {
                report.warningsCount++;
                report.issues.push_back({ MapIssueSeverity::Warning, ts.GetName(), "Tileset has tilecount of 0." });
            }
            if (assetMgr && !ts.GetTextureId().empty()) {
                if (!assetMgr->HasTexture(ts.GetTextureId())) {
                    report.warningsCount++;
                    report.issues.push_back({ MapIssueSeverity::Warning, ts.GetName(),
                                              "Tileset texture '" + ts.GetTextureId() + "' not pre-registered in AMS." });
                }
            }
        }
    }

    // 3. Validate Layers & GIDs
    if (tilemap.GetLayers().empty()) {
        report.warningsCount++;
        report.issues.push_back({ MapIssueSeverity::Warning, "Layers", "Map contains zero tile layers." });
    } else {
        for (const auto& layer : tilemap.GetLayers()) {
            if (layer.GetWidth() != tilemap.GetWidth() || layer.GetHeight() != tilemap.GetHeight()) {
                report.errorsCount++;
                report.issues.push_back({ MapIssueSeverity::Error, layer.GetName(), "Layer dimensions do not match map dimensions." });
            }

            // Check for invalid GIDs
            size_t invalidGids = 0;
            for (int r = 0; r < layer.GetHeight(); ++r) {
                for (int c = 0; c < layer.GetWidth(); ++c) {
                    int gid = layer.GetTileGid(c, r);
                    if (gid > 0 && tilemap.FindTilesetForGid(gid) == nullptr) {
                        invalidGids++;
                    }
                }
            }

            if (invalidGids > 0) {
                report.errorsCount++;
                report.issues.push_back({ MapIssueSeverity::Error, layer.GetName(),
                                          "Layer contains " + std::to_string(invalidGids) + " unresolvable GIDs." });
            }
        }
    }

    // 4. Validate Environmental Objects
    for (const auto& obj : tilemap.GetObjects()) {
        if (!obj.HasCollision()) {
            report.warningsCount++;
            report.issues.push_back({ MapIssueSeverity::Warning, obj.GetName(), "Object has no collision shape defined." });
        }
    }

    Logger::Info("MAP_VALIDATOR", "Audited map '" + report.mapName + "': " +
                 std::to_string(report.errorsCount) + " errors, " +
                 std::to_string(report.warningsCount) + " warnings.");

    return report;
}

} // namespace wildbound::world


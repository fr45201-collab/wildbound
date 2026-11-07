#pragma once

#include "Tilemap.hpp"
#include <string>
#include <memory>

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::world {

/**
 * Loads Tiled Map Editor TMX (XML+CSV) and JSON maps into runtime Tilemap structures.
 */
class MapLoader {
public:
    static std::unique_ptr<Tilemap> LoadMap(const std::string& mapFilePath, resources::AssetManager* assetMgr = nullptr);
    static std::unique_ptr<Tilemap> LoadMap(const std::string& mapFilePath, resources::AssetManager& assetMgr) { return LoadMap(mapFilePath, &assetMgr); }

    static std::unique_ptr<Tilemap> LoadFromTmx(const std::string& tmxFilePath, resources::AssetManager* assetMgr = nullptr);
    static std::unique_ptr<Tilemap> LoadFromTmx(const std::string& tmxFilePath, resources::AssetManager& assetMgr) { return LoadFromTmx(tmxFilePath, &assetMgr); }

    static std::unique_ptr<Tilemap> LoadFromJson(const std::string& jsonFilePath, resources::AssetManager* assetMgr = nullptr);
    static std::unique_ptr<Tilemap> LoadFromJson(const std::string& jsonFilePath, resources::AssetManager& assetMgr) { return LoadFromJson(jsonFilePath, &assetMgr); }

private:
    static std::vector<int> ParseCsvData(const std::string& csv);
    static std::string CleanXmlText(const std::string& raw);
};

} // namespace wildbound::world


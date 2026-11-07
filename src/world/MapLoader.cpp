#include "MapLoader.hpp"
#include "../core/Logger.hpp"
#include "../resources/AssetManager.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

using wildbound::core::Logger;
namespace fs = std::filesystem;

namespace wildbound::world {

std::vector<int> MapLoader::ParseCsvData(const std::string& csv) {
    std::vector<int> data;
    std::stringstream ss(csv);
    std::string token;

    while (std::getline(ss, token, ',')) {
        // Strip whitespace and newlines
        token.erase(std::remove_if(token.begin(), token.end(), [](unsigned char c) {
            return std::isspace(c);
        }), token.end());

        if (!token.empty()) {
            try {
                data.push_back(std::stoi(token));
            }
            catch (...) {
                data.push_back(0);
            }
        }
    }
    return data;
}

std::unique_ptr<Tilemap> MapLoader::LoadMap(const std::string& mapFilePath, resources::AssetManager* assetMgr) {
    if (!fs::exists(mapFilePath)) {
        Logger::Error("MAP_LOADER", "Map file does not exist: " + mapFilePath);
        return nullptr;
    }

    std::string ext = fs::path(mapFilePath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (ext == ".json" || ext == ".tmj") {
        return LoadFromJson(mapFilePath, assetMgr);
    } else if (ext == ".tmx") {
        return LoadFromTmx(mapFilePath, assetMgr);
    } else {
        Logger::Error("MAP_LOADER", "Unsupported map format extension: " + ext);
        return nullptr;
    }
}

std::unique_ptr<Tilemap> MapLoader::LoadFromJson(const std::string& jsonFilePath, resources::AssetManager* assetMgr) {
    try {
        std::ifstream file(jsonFilePath);
        if (!file.is_open()) {
            Logger::Error("MAP_LOADER", "Could not open map JSON: " + jsonFilePath);
            return nullptr;
        }

        nlohmann::json root;
        file >> root;

        std::string name = fs::path(jsonFilePath).stem().string();
        int width = root.value("width", 0);
        int height = root.value("height", 0);
        int tileWidth = root.value("tilewidth", 32);
        int tileHeight = root.value("tileheight", 32);

        auto tilemap = std::make_unique<Tilemap>(name, width, height, tileWidth, tileHeight);

        // 1. Parse Tilesets
        if (root.contains("tilesets") && root["tilesets"].is_array()) {
            for (const auto& tsItem : root["tilesets"]) {
                int firstGid = tsItem.value("firstgid", 1);
                std::string tsName = tsItem.value("name", "tileset");
                int tw = tsItem.value("tilewidth", tileWidth);
                int th = tsItem.value("tileheight", tileHeight);
                int cols = tsItem.value("columns", 1);
                int tileCount = tsItem.value("tilecount", 0);
                std::string image = tsItem.value("image", "");
                int spacing = tsItem.value("spacing", 0);
                int margin = tsItem.value("margin", 0);

                Tileset tileset(firstGid, tsName, tw, th, cols, tileCount, image, spacing, margin);

                // Register texture with AssetManager if available
                if (!image.empty()) {
                    tileset.SetTextureId(tsName);
                    if (assetMgr) {
                        assetMgr->GetTexture(tsName); // Preload texture into cache
                    }
                }

                tilemap->AddTileset(std::move(tileset));
            }
        }

        // 2. Parse Layers
        if (root.contains("layers") && root["layers"].is_array()) {
            for (const auto& lItem : root["layers"]) {
                std::string lType = lItem.value("type", "tilelayer");
                std::string lName = lItem.value("name", "layer");
                int lId = lItem.value("id", 0);
                bool visible = lItem.value("visible", true);
                float opacity = lItem.value("opacity", 1.0f);

                if (lType == "tilelayer") {
                    int lw = lItem.value("width", width);
                    int lh = lItem.value("height", height);
                    TileLayer layer(lId, lName, lw, lh, opacity, visible);

                    if (lItem.contains("data")) {
                        if (lItem["data"].is_array()) {
                            std::vector<int> data = lItem["data"].get<std::vector<int>>();
                            layer.SetData(std::move(data));
                        } else if (lItem["data"].is_string()) {
                            std::string csvStr = lItem["data"].get<std::string>();
                            layer.SetData(ParseCsvData(csvStr));
                        }
                    }
                    tilemap->AddLayer(std::move(layer));
                }
                else if (lType == "objectgroup") {
                    // Parse Environmental Objects
                    if (lItem.contains("objects") && lItem["objects"].is_array()) {
                        for (const auto& objItem : lItem["objects"]) {
                            int objId = objItem.value("id", 0);
                            std::string objName = objItem.value("name", "object");
                            std::string objType = objItem.value("type", "prop");
                            float x = objItem.value("x", 0.0f);
                            float y = objItem.value("y", 0.0f);
                            float w = objItem.value("width", 32.0f);
                            float h = objItem.value("height", 32.0f);
                            float rot = objItem.value("rotation", 0.0f);

                            // In Tiled, object position (x, y) for tile objects is bottom-left
                            Vector2 pos{ x, y };
                            Vector2 size{ w, h };
                            Vector2 origin{ w * 0.5f, h }; // Standard entity anchor at bottom-center

                            std::string textureId = (objType == "spawn" || objType == "trigger" || objType == "waypoint") ? "" : ("tex_" + objType);
                            Rectangle srcRect{ 0.0f, 0.0f, w, h };

                            EnvironmentObject envObj(objId, objName, objType, pos, size, textureId, srcRect, origin, 1.0f, rot);

                            // Parse custom properties (collision)
                            if (objType != "spawn" && objItem.contains("properties") && objItem["properties"].is_array()) {
                                bool hasCollisionProp = false;
                                CollisionBox cbox;
                                cbox.bounds = Rectangle{ x - origin.x, y - origin.y, w, h };
                                for (const auto& prop : objItem["properties"]) {
                                    std::string pName = prop.value("name", "");
                                    if (pName == "collision") {
                                        bool isSolid = prop.value("value", true);
                                        cbox.type = isSolid ? CollisionType::Solid : CollisionType::None;
                                        hasCollisionProp = true;
                                    } else if (pName == "collision_type") {
                                        std::string typeStr = prop.value("value", "solid");
                                        cbox.type = CollisionBox::StringToType(typeStr);
                                        hasCollisionProp = true;
                                    } else if (pName == "collision_width") {
                                        float cw = prop.value("value", w);
                                        cbox.bounds.width = cw;
                                    } else if (pName == "collision_height") {
                                        float ch = prop.value("value", h * 0.4f);
                                        cbox.bounds.height = ch;
                                    }
                                }
                                if (hasCollisionProp && cbox.IsSolid()) {
                                    envObj.SetCollision(cbox);
                                }
                            }
                            tilemap->AddObject(std::move(envObj));
                        }
                    }
                }
            }
        }

        Logger::Info("MAP_LOADER", "Loaded Tiled JSON map: " + name + " (" +
                     std::to_string(width) + "x" + std::to_string(height) + " tiles, " +
                     std::to_string(tilemap->GetLayerCount()) + " layers, " +
                     std::to_string(tilemap->GetObjects().size()) + " objects)");
        return tilemap;
    }
    catch (const std::exception& ex) {
        Logger::Error("MAP_LOADER", "Failed loading Tiled JSON map: " + std::string(ex.what()));
        return nullptr;
    }
}

std::unique_ptr<Tilemap> MapLoader::LoadFromTmx(const std::string& tmxFilePath, resources::AssetManager* assetMgr) {
    try {
        std::ifstream file(tmxFilePath);
        if (!file.is_open()) {
            Logger::Error("MAP_LOADER", "Could not open TMX file: " + tmxFilePath);
            return nullptr;
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string name = fs::path(tmxFilePath).stem().string();

        // Helper lambda to extract attribute value from an XML tag string
        auto GetAttr = [](const std::string& tag, const std::string& attrName) -> std::string {
            std::string needle = attrName + "=\"";
            size_t pos = tag.find(needle);
            if (pos == std::string::npos) return "";
            size_t start = pos + needle.length();
            size_t end = tag.find('"', start);
            if (end == std::string::npos) return "";
            return tag.substr(start, end - start);
        };

        // 1. Parse <map> tag
        size_t mapTagPos = content.find("<map");
        if (mapTagPos == std::string::npos) {
            Logger::Error("MAP_LOADER", "Invalid TMX: missing <map> tag in " + tmxFilePath);
            return nullptr;
        }
        size_t mapTagEnd = content.find('>', mapTagPos);
        std::string mapTag = content.substr(mapTagPos, mapTagEnd - mapTagPos + 1);

        int width = std::stoi(GetAttr(mapTag, "width"));
        int height = std::stoi(GetAttr(mapTag, "height"));
        int tileWidth = std::stoi(GetAttr(mapTag, "tilewidth"));
        int tileHeight = std::stoi(GetAttr(mapTag, "tileheight"));

        auto tilemap = std::make_unique<Tilemap>(name, width, height, tileWidth, tileHeight);

        // 2. Parse <tileset> tags
        size_t tsPos = 0;
        while ((tsPos = content.find("<tileset", tsPos)) != std::string::npos) {
            size_t tsEnd = content.find('>', tsPos);
            std::string tsTag = content.substr(tsPos, tsEnd - tsPos + 1);

            int firstGid = std::stoi(GetAttr(tsTag, "firstgid"));
            std::string tsName = GetAttr(tsTag, "name");
            int tw = GetAttr(tsTag, "tilewidth").empty() ? tileWidth : std::stoi(GetAttr(tsTag, "tilewidth"));
            int th = GetAttr(tsTag, "tileheight").empty() ? tileHeight : std::stoi(GetAttr(tsTag, "tileheight"));
            int cols = GetAttr(tsTag, "columns").empty() ? 8 : std::stoi(GetAttr(tsTag, "columns"));
            int count = GetAttr(tsTag, "tilecount").empty() ? 64 : std::stoi(GetAttr(tsTag, "tilecount"));

            // Check if <image> child exists
            std::string imageSource;
            size_t nextTs = content.find("</tileset>", tsPos);
            size_t imgPos = content.find("<image", tsPos);
            if (imgPos != std::string::npos && (nextTs == std::string::npos || imgPos < nextTs)) {
                size_t imgEnd = content.find('>', imgPos);
                std::string imgTag = content.substr(imgPos, imgEnd - imgPos + 1);
                imageSource = GetAttr(imgTag, "source");
            }

            Tileset tileset(firstGid, tsName, tw, th, cols, count, imageSource);
            tileset.SetTextureId(tsName);

            // Preload texture into AssetManager cache if available
            if (!imageSource.empty()) {
                if (assetMgr) {
                    assetMgr->GetTexture(tsName);
                }
            }

            tilemap->AddTileset(std::move(tileset));
            tsPos = (nextTs != std::string::npos) ? (nextTs + 10) : (tsEnd + 1);
        }

        // 3. Parse <layer> tags with CSV data
        size_t layerPos = 0;
        while ((layerPos = content.find("<layer", layerPos)) != std::string::npos) {
            size_t layerEnd = content.find('>', layerPos);
            std::string lTag = content.substr(layerPos, layerEnd - layerPos + 1);

            int lId = GetAttr(lTag, "id").empty() ? 0 : std::stoi(GetAttr(lTag, "id"));
            std::string lName = GetAttr(lTag, "name");
            int lw = GetAttr(lTag, "width").empty() ? width : std::stoi(GetAttr(lTag, "width"));
            int lh = GetAttr(lTag, "height").empty() ? height : std::stoi(GetAttr(lTag, "height"));
            float opacity = GetAttr(lTag, "opacity").empty() ? 1.0f : std::stof(GetAttr(lTag, "opacity"));
            bool visible = GetAttr(lTag, "visible") != "0";

            TileLayer layer(lId, lName, lw, lh, opacity, visible);

            // Find <data encoding="csv">...</data>
            size_t dataStart = content.find("<data", layerEnd);
            size_t dataTagEnd = content.find('>', dataStart);
            size_t dataClose = content.find("</data>", dataTagEnd);

            if (dataStart != std::string::npos && dataClose != std::string::npos) {
                std::string csv = content.substr(dataTagEnd + 1, dataClose - (dataTagEnd + 1));
                layer.SetData(ParseCsvData(csv));
            }

            tilemap->AddLayer(std::move(layer));
            layerPos = dataClose + 7;
        }

        // 4. Parse <objectgroup> and <object>
        size_t objGroupPos = 0;
        while ((objGroupPos = content.find("<objectgroup", objGroupPos)) != std::string::npos) {
            size_t objGroupEnd = content.find("</objectgroup>", objGroupPos);
            if (objGroupEnd == std::string::npos) break;

            size_t objPos = objGroupPos;
            while ((objPos = content.find("<object", objPos)) != std::string::npos && objPos < objGroupEnd) {
                size_t nextTagClose = content.find('>', objPos);
                std::string objTag = content.substr(objPos, nextTagClose - objPos + 1);

                int objId = GetAttr(objTag, "id").empty() ? 0 : std::stoi(GetAttr(objTag, "id"));
                std::string objName = GetAttr(objTag, "name");
                std::string objType = GetAttr(objTag, "type");
                float x = GetAttr(objTag, "x").empty() ? 0.0f : std::stof(GetAttr(objTag, "x"));
                float y = GetAttr(objTag, "y").empty() ? 0.0f : std::stof(GetAttr(objTag, "y"));
                float w = GetAttr(objTag, "width").empty() ? 32.0f : std::stof(GetAttr(objTag, "width"));
                float h = GetAttr(objTag, "height").empty() ? 32.0f : std::stof(GetAttr(objTag, "height"));

                Vector2 pos{ x, y };
                Vector2 size{ w, h };
                Vector2 origin{ w * 0.5f, h };

                std::string texKey = (objType == "spawn" || objType == "trigger" || objType == "waypoint") ? "" : ("tex_" + objType);
                Rectangle srcRect{ 0.0f, 0.0f, w, h };
                EnvironmentObject envObj(objId, objName, objType, pos, size, texKey, srcRect, origin);

                // Check collision properties in object body
                if (objType != "spawn") {
                    size_t objClose = content.find("</object>", objPos);
                    if (objClose != std::string::npos && objClose < objGroupEnd) {
                        std::string objBody = content.substr(objPos, objClose - objPos + 9);
                        CollisionBox cbox;
                        cbox.bounds = Rectangle{ x - origin.x, y - origin.y + h * 0.5f, w, h * 0.5f };
                        cbox.type = CollisionType::Solid;
                        envObj.SetCollision(cbox);
                    }
                }

                tilemap->AddObject(std::move(envObj));
                objPos = nextTagClose + 1;
            }
            objGroupPos = objGroupEnd + 14;
        }

        Logger::Info("MAP_LOADER", "Loaded Tiled TMX map: " + name + " (" +
                     std::to_string(width) + "x" + std::to_string(height) + " tiles, " +
                     std::to_string(tilemap->GetLayerCount()) + " layers, " +
                     std::to_string(tilemap->GetObjects().size()) + " objects)");
        return tilemap;
    }
    catch (const std::exception& ex) {
        Logger::Error("MAP_LOADER", "Failed parsing TMX file: " + std::string(ex.what()));
        return nullptr;
    }
}

} // namespace wildbound::world


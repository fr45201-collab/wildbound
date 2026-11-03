#pragma once

#include "Tileset.hpp"
#include "TileLayer.hpp"
#include "EnvironmentObject.hpp"
#include <string>
#include <vector>
#include <memory>

namespace wildbound::world {

/**
 * Data container for complete map data: layers, tilesets, objects, and world bounds.
 */
class Tilemap {
public:
    Tilemap();
    Tilemap(const std::string& name, int width, int height, int tileWidth = 32, int tileHeight = 32);

    void AddTileset(Tileset tileset);
    void AddLayer(TileLayer layer);
    void AddObject(EnvironmentObject obj);

    const Tileset* FindTilesetForGid(int gid) const;
    const TileLayer* GetLayer(const std::string& name) const;
    TileLayer* GetLayer(const std::string& name);
    const TileLayer* GetLayerAt(size_t index) const;
    size_t GetLayerCount() const { return m_layers.size(); }

    const std::vector<TileLayer>& GetLayers() const { return m_layers; }
    std::vector<TileLayer>& GetLayers() { return m_layers; }

    const std::vector<Tileset>& GetTilesets() const { return m_tilesets; }
    const std::vector<EnvironmentObject>& GetObjects() const { return m_objects; }
    std::vector<EnvironmentObject>& GetObjects() { return m_objects; }

    // Map properties
    const std::string& GetName() const { return m_name; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetTileWidth() const { return m_tileWidth; }
    int GetTileHeight() const { return m_tileHeight; }

    int GetWorldWidth() const { return m_width * m_tileWidth; }
    int GetWorldHeight() const { return m_height * m_tileHeight; }
    Rectangle GetWorldBounds() const;

    Vector2 WorldToTile(Vector2 worldPos) const;
    Vector2 TileToWorld(int col, int row) const;

    void SetProperty(const std::string& key, const std::string& value);
    std::string GetProperty(const std::string& key) const;

private:
    std::string m_name;
    int m_width = 0;
    int m_height = 0;
    int m_tileWidth = 32;
    int m_tileHeight = 32;

    std::vector<Tileset> m_tilesets;
    std::vector<TileLayer> m_layers;
    std::vector<EnvironmentObject> m_objects;
    std::unordered_map<std::string, std::string> m_properties;
};

} // namespace wildbound::world


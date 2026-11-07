#include "Tilemap.hpp"
#include <cmath>

namespace wildbound::world {

Tilemap::Tilemap() = default;

Tilemap::Tilemap(const std::string& name, int width, int height, int tileWidth, int tileHeight)
    : m_name(name)
    , m_width(width)
    , m_height(height)
    , m_tileWidth(tileWidth > 0 ? tileWidth : 32)
    , m_tileHeight(tileHeight > 0 ? tileHeight : 32)
{
}

void Tilemap::AddTileset(Tileset tileset) {
    m_tilesets.push_back(std::move(tileset));
}

void Tilemap::AddLayer(TileLayer layer) {
    m_layers.push_back(std::move(layer));
}

void Tilemap::AddObject(EnvironmentObject obj) {
    m_objects.push_back(std::move(obj));
}

const Tileset* Tilemap::FindTilesetForGid(int gid) const {
    if (gid <= 0) return nullptr;

    const Tileset* bestMatch = nullptr;
    int highestFirstGid = -1;

    for (const auto& ts : m_tilesets) {
        if (gid >= ts.GetFirstGid() && ts.GetFirstGid() > highestFirstGid) {
            if (ts.ContainsGid(gid)) {
                bestMatch = &ts;
                highestFirstGid = ts.GetFirstGid();
            }
        }
    }
    return bestMatch;
}

const TileLayer* Tilemap::GetLayer(const std::string& name) const {
    for (const auto& l : m_layers) {
        if (l.GetName() == name) return &l;
    }
    return nullptr;
}

TileLayer* Tilemap::GetLayer(const std::string& name) {
    for (auto& l : m_layers) {
        if (l.GetName() == name) return &l;
    }
    return nullptr;
}

const TileLayer* Tilemap::GetLayerAt(size_t index) const {
    if (index < m_layers.size()) {
        return &m_layers[index];
    }
    return nullptr;
}

Rectangle Tilemap::GetWorldBounds() const {
    return Rectangle{
        0.0f,
        0.0f,
        static_cast<float>(GetWorldWidth()),
        static_cast<float>(GetWorldHeight())
    };
}

Vector2 Tilemap::WorldToTile(Vector2 worldPos) const {
    float tw = m_tileWidth > 0 ? static_cast<float>(m_tileWidth) : 32.0f;
    float th = m_tileHeight > 0 ? static_cast<float>(m_tileHeight) : 32.0f;
    return Vector2{
        std::floor(worldPos.x / tw),
        std::floor(worldPos.y / th)
    };
}

Vector2 Tilemap::TileToWorld(int col, int row) const {
    return Vector2{
        static_cast<float>(col * m_tileWidth),
        static_cast<float>(row * m_tileHeight)
    };
}

Vector2 Tilemap::GetSpawnPosition(const std::string& spawnName) const {
    for (const auto& obj : m_objects) {
        if (obj.GetName() == spawnName || obj.GetType() == "spawn") {
            return obj.GetPosition();
        }
    }
    // Default fallback: near center-left pathway
    return Vector2{ 256.0f, 960.0f };
}

void Tilemap::SetProperty(const std::string& key, const std::string& value) {
    m_properties[key] = value;
}

std::string Tilemap::GetProperty(const std::string& key) const {
    auto it = m_properties.find(key);
    if (it != m_properties.end()) {
        return it->second;
    }
    return "";
}

} // namespace wildbound::world


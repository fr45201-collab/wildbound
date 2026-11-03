#include "Tileset.hpp"
#include <algorithm>

namespace wildbound::world {

Tileset::Tileset() = default;

Tileset::Tileset(int firstGid, const std::string& name,
                 int tileWidth, int tileHeight,
                 int columns, int tileCount,
                 const std::string& imageSource,
                 int spacing, int margin)
    : m_firstGid(firstGid)
    , m_name(name)
    , m_tileWidth(tileWidth > 0 ? tileWidth : 32)
    , m_tileHeight(tileHeight > 0 ? tileHeight : 32)
    , m_columns(columns > 0 ? columns : 1)
    , m_tileCount(tileCount >= 0 ? tileCount : 0)
    , m_spacing(spacing)
    , m_margin(margin)
    , m_imageSource(imageSource)
    , m_textureId(name)
{
}

bool Tileset::ContainsGid(int gid) const {
    if (gid < m_firstGid) return false;
    if (m_tileCount > 0 && gid >= m_firstGid + m_tileCount) return false;
    return true;
}

int Tileset::GidToLocalId(int gid) const {
    return gid - m_firstGid;
}

Rectangle Tileset::GetTileSourceRect(int gid) const {
    if (!ContainsGid(gid) || m_columns <= 0) {
        return Rectangle{ 0.0f, 0.0f, 0.0f, 0.0f };
    }

    int localId = GidToLocalId(gid);
    int col = localId % m_columns;
    int row = localId / m_columns;

    float x = static_cast<float>(m_margin + col * (m_tileWidth + m_spacing));
    float y = static_cast<float>(m_margin + row * (m_tileHeight + m_spacing));

    return Rectangle{
        x,
        y,
        static_cast<float>(m_tileWidth),
        static_cast<float>(m_tileHeight)
    };
}

void Tileset::SetTileProperty(int localId, const std::string& key, const std::string& value) {
    m_tileProperties[localId][key] = value;
}

std::string Tileset::GetTileProperty(int localId, const std::string& key) const {
    auto it = m_tileProperties.find(localId);
    if (it != m_tileProperties.end()) {
        auto propIt = it->second.find(key);
        if (propIt != it->second.end()) {
            return propIt->second;
        }
    }
    return "";
}

bool Tileset::HasTileProperty(int localId, const std::string& key) const {
    auto it = m_tileProperties.find(localId);
    if (it != m_tileProperties.end()) {
        return it->second.find(key) != it->second.end();
    }
    return false;
}

} // namespace wildbound::world


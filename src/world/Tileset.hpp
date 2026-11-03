#pragma once

#include <raylib.h>
#include <string>
#include <unordered_map>

namespace wildbound::world {

/**
 * Manages tileset geometry, source textures, GID mapping, and tile properties.
 */
class Tileset {
public:
    Tileset();
    Tileset(int firstGid, const std::string& name,
            int tileWidth, int tileHeight,
            int columns, int tileCount,
            const std::string& imageSource,
            int spacing = 0, int margin = 0);

    bool ContainsGid(int gid) const;
    Rectangle GetTileSourceRect(int gid) const;
    int GidToLocalId(int gid) const;

    // Attributes
    int GetFirstGid() const { return m_firstGid; }
    const std::string& GetName() const { return m_name; }
    int GetTileWidth() const { return m_tileWidth; }
    int GetTileHeight() const { return m_tileHeight; }
    int GetColumns() const { return m_columns; }
    int GetTileCount() const { return m_tileCount; }
    const std::string& GetImageSource() const { return m_imageSource; }
    const std::string& GetTextureId() const { return m_textureId; }
    void SetTextureId(const std::string& texKey) { m_textureId = texKey; }

    // Tile properties (e.g. collision, terrain type)
    void SetTileProperty(int localId, const std::string& key, const std::string& value);
    std::string GetTileProperty(int localId, const std::string& key) const;
    bool HasTileProperty(int localId, const std::string& key) const;

private:
    int m_firstGid = 1;
    std::string m_name;
    int m_tileWidth = 32;
    int m_tileHeight = 32;
    int m_columns = 1;
    int m_tileCount = 0;
    int m_spacing = 0;
    int m_margin = 0;

    std::string m_imageSource;
    std::string m_textureId;

    // LocalId -> (PropertyKey -> Value)
    std::unordered_map<int, std::unordered_map<std::string, std::string>> m_tileProperties;
};

} // namespace wildbound::world


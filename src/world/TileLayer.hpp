#pragma once

#include <string>
#include <vector>

namespace wildbound::world {

/**
 * 2D Grid Layer containing GID tile references.
 */
class TileLayer {
public:
    TileLayer();
    TileLayer(int id, const std::string& name, int width, int height, float opacity = 1.0f, bool visible = true);

    int GetTileGid(int col, int row) const;
    void SetTileGid(int col, int row, int gid);
    void SetData(std::vector<int> data);

    int GetId() const { return m_id; }
    const std::string& GetName() const { return m_name; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    size_t GetTotalTiles() const { return static_cast<size_t>(m_width * m_height); }
    float GetOpacity() const { return m_opacity; }
    void SetOpacity(float opacity) { m_opacity = opacity; }
    bool IsVisible() const { return m_visible; }
    void SetVisible(bool visible) { m_visible = visible; }

    const std::vector<int>& GetData() const { return m_data; }

private:
    int m_id = 0;
    std::string m_name;
    int m_width = 0;
    int m_height = 0;
    float m_opacity = 1.0f;
    bool m_visible = true;
    std::vector<int> m_data;
};

} // namespace wildbound::world


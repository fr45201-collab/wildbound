#include "TileLayer.hpp"

namespace wildbound::world {

TileLayer::TileLayer() = default;

TileLayer::TileLayer(int id, const std::string& name, int width, int height, float opacity, bool visible)
    : m_id(id)
    , m_name(name)
    , m_width(width)
    , m_height(height)
    , m_opacity(opacity)
    , m_visible(visible)
{
    if (m_width > 0 && m_height > 0) {
        m_data.resize(static_cast<size_t>(m_width) * static_cast<size_t>(m_height), 0);
    }
}

int TileLayer::GetTileGid(int col, int row) const {
    if (col < 0 || col >= m_width || row < 0 || row >= m_height) {
        return 0; // Out of bounds tile is empty
    }
    size_t idx = static_cast<size_t>(row) * static_cast<size_t>(m_width) + static_cast<size_t>(col);
    if (idx < m_data.size()) {
        return m_data[idx];
    }
    return 0;
}

void TileLayer::SetTileGid(int col, int row, int gid) {
    if (col >= 0 && col < m_width && row >= 0 && row < m_height) {
        size_t idx = static_cast<size_t>(row) * static_cast<size_t>(m_width) + static_cast<size_t>(col);
        if (idx < m_data.size()) {
            m_data[idx] = gid;
        }
    }
}

void TileLayer::SetData(std::vector<int> data) {
    m_data = std::move(data);
}

} // namespace wildbound::world


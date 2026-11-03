#include "SpriteSheet.hpp"
#include <algorithm>

namespace wildbound::resources {

SpriteSheet::SpriteSheet() {
    ComputeDefaultOrigin();
}

SpriteSheet::SpriteSheet(const std::string& id, const std::string& textureId,
                         int frameWidth, int frameHeight,
                         int columns, int rows, int frameCount,
                         SpriteOrigin origin)
    : m_id(id)
    , m_textureId(textureId)
    , m_frameWidth(frameWidth > 0 ? frameWidth : 32)
    , m_frameHeight(frameHeight > 0 ? frameHeight : 32)
    , m_columns(columns > 0 ? columns : 1)
    , m_rows(rows > 0 ? rows : 1)
    , m_frameCount(frameCount > 0 ? frameCount : (columns * rows))
    , m_originType(origin)
{
    ComputeDefaultOrigin();
}

void SpriteSheet::ComputeDefaultOrigin() {
    switch (m_originType) {
        case SpriteOrigin::TopLeft:
            m_defaultOrigin = Vector2{ 0.0f, 0.0f };
            break;
        case SpriteOrigin::Center:
            m_defaultOrigin = Vector2{ static_cast<float>(m_frameWidth) * 0.5f, static_cast<float>(m_frameHeight) * 0.5f };
            break;
        case SpriteOrigin::BottomCenter:
            m_defaultOrigin = Vector2{ static_cast<float>(m_frameWidth) * 0.5f, static_cast<float>(m_frameHeight) };
            break;
        default:
            m_defaultOrigin = Vector2{ 0.0f, 0.0f };
            break;
    }
}

Rectangle SpriteSheet::GetFrameRect(int frameIndex) const {
    if (!m_customFrames.empty()) {
        int idx = std::clamp(frameIndex, 0, static_cast<int>(m_customFrames.size()) - 1);
        return m_customFrames[idx].rect;
    }

    int count = GetFrameCount();
    if (count <= 0) {
        return Rectangle{ 0.0f, 0.0f, static_cast<float>(m_frameWidth), static_cast<float>(m_frameHeight) };
    }

    int clampedIndex = std::clamp(frameIndex, 0, count - 1);
    int col = clampedIndex % m_columns;
    int row = clampedIndex / m_columns;

    float x = static_cast<float>(m_margin + col * (m_frameWidth + m_spacing));
    float y = static_cast<float>(m_margin + row * (m_frameHeight + m_spacing));

    return Rectangle{ x, y, static_cast<float>(m_frameWidth), static_cast<float>(m_frameHeight) };
}

Vector2 SpriteSheet::GetFrameOrigin(int frameIndex) const {
    if (!m_customFrames.empty()) {
        int idx = std::clamp(frameIndex, 0, static_cast<int>(m_customFrames.size()) - 1);
        return m_customFrames[idx].origin;
    }
    return m_defaultOrigin;
}

int SpriteSheet::GetFrameCount() const {
    if (!m_customFrames.empty()) {
        return static_cast<int>(m_customFrames.size());
    }
    return m_frameCount;
}

void SpriteSheet::SetCustomFrames(const std::vector<SpriteFrame>& frames) {
    m_customFrames = frames;
    if (!m_customFrames.empty()) {
        m_frameCount = static_cast<int>(m_customFrames.size());
    }
}

std::string_view SpriteSheet::OriginToString(SpriteOrigin origin) {
    switch (origin) {
        case SpriteOrigin::TopLeft:      return "top_left";
        case SpriteOrigin::Center:       return "center";
        case SpriteOrigin::BottomCenter: return "bottom_center";
        case SpriteOrigin::Custom:       return "custom";
        default:                         return "center";
    }
}

SpriteOrigin SpriteSheet::StringToOrigin(std::string_view str) {
    if (str == "top_left")      return SpriteOrigin::TopLeft;
    if (str == "center")        return SpriteOrigin::Center;
    if (str == "bottom_center") return SpriteOrigin::BottomCenter;
    if (str == "custom")        return SpriteOrigin::Custom;
    return SpriteOrigin::Center;
}

nlohmann::json SpriteSheet::ToJson() const {
    nlohmann::json j;
    j["id"] = m_id;
    j["texture"] = m_textureId;
    j["frame_width"] = m_frameWidth;
    j["frame_height"] = m_frameHeight;
    j["columns"] = m_columns;
    j["rows"] = m_rows;
    j["frame_count"] = m_frameCount;
    j["spacing"] = m_spacing;
    j["margin"] = m_margin;
    j["origin"] = OriginToString(m_originType);
    return j;
}

SpriteSheet SpriteSheet::FromJson(const nlohmann::json& j) {
    std::string id = j.value("id", "");
    std::string texture = j.value("texture", "");
    int fw = j.value("frame_width", 32);
    int fh = j.value("frame_height", 32);
    int cols = j.value("columns", 1);
    int rows = j.value("rows", 1);
    int count = j.value("frame_count", cols * rows);
    SpriteOrigin origin = StringToOrigin(j.value("origin", "center"));

    SpriteSheet sheet(id, texture, fw, fh, cols, rows, count, origin);
    sheet.m_spacing = j.value("spacing", 0);
    sheet.m_margin = j.value("margin", 0);

    return sheet;
}

} // namespace wildbound::resources


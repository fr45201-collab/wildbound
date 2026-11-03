#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace wildbound::resources {

enum class SpriteOrigin {
    TopLeft = 0,
    Center,
    BottomCenter,
    Custom
};

struct SpriteFrame {
    Rectangle rect{ 0.0f, 0.0f, 0.0f, 0.0f };
    Vector2 origin{ 0.0f, 0.0f };
    float duration = 0.1f;
};

/**
 * Metadata and frame computation engine for grid-based and variable-layout spritesheets.
 */
class SpriteSheet {
public:
    SpriteSheet();
    explicit SpriteSheet(const std::string& id, const std::string& textureId,
                         int frameWidth, int frameHeight,
                         int columns = 1, int rows = 1, int frameCount = 1,
                         SpriteOrigin origin = SpriteOrigin::Center);

    // Frame selection
    Rectangle GetFrameRect(int frameIndex) const;
    Vector2 GetFrameOrigin(int frameIndex) const;
    int GetFrameCount() const;

    // Attributes
    const std::string& GetId() const { return m_id; }
    const std::string& GetTextureId() const { return m_textureId; }
    int GetFrameWidth() const { return m_frameWidth; }
    int GetFrameHeight() const { return m_frameHeight; }
    int GetColumns() const { return m_columns; }
    int GetRows() const { return m_rows; }
    SpriteOrigin GetOriginType() const { return m_originType; }

    void SetCustomFrames(const std::vector<SpriteFrame>& frames);
    bool HasCustomFrames() const { return !m_customFrames.empty(); }

    nlohmann::json ToJson() const;
    static SpriteSheet FromJson(const nlohmann::json& j);

    static std::string_view OriginToString(SpriteOrigin origin);
    static SpriteOrigin StringToOrigin(std::string_view str);

private:
    void ComputeDefaultOrigin();

    std::string m_id;
    std::string m_textureId;
    int m_frameWidth = 32;
    int m_frameHeight = 32;
    int m_columns = 1;
    int m_rows = 1;
    int m_frameCount = 1;
    int m_spacing = 0;
    int m_margin = 0;

    SpriteOrigin m_originType = SpriteOrigin::Center;
    Vector2 m_defaultOrigin{ 16.0f, 16.0f };

    std::vector<SpriteFrame> m_customFrames;
};

} // namespace wildbound::resources


#pragma once

#include <raylib.h>

namespace wildbound::graphics {

/**
 * RAII Wrapper for raylib RenderTexture2D with pixel-art point filtering.
 */
class RenderTexture {
public:
    RenderTexture();
    explicit RenderTexture(int width, int height);
    ~RenderTexture();

    RenderTexture(const RenderTexture&) = delete;
    RenderTexture& operator=(const RenderTexture&) = delete;

    RenderTexture(RenderTexture&& other) noexcept;
    RenderTexture& operator=(RenderTexture&& other) noexcept;

    bool Load(int width, int height);
    void Unload();

    void Begin();
    void End();

    bool IsValid() const { return m_target.id != 0; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    const RenderTexture2D& GetRaw() const { return m_target; }
    const Texture2D& GetTexture() const { return m_target.texture; }

private:
    RenderTexture2D m_target{};
    int m_width = 0;
    int m_height = 0;
    bool m_isBound = false;
};

} // namespace wildbound::graphics


#pragma once

#include <raylib.h>

namespace wildbound::graphics {

/**
 * Manages virtual resolution, aspect ratio preservation, letterbox centering,
 * and bi-directional Screen <-> Virtual coordinate conversions.
 */
class Viewport {
public:
    Viewport(int virtualWidth = 640, int virtualHeight = 360,
             int windowWidth = 1280, int windowHeight = 720,
             bool preferIntegerScaling = false);

    void Update(int newWindowWidth, int newWindowHeight);
    void SetVirtualResolution(int virtualWidth, int virtualHeight);
    void SetPreferIntegerScaling(bool preferInteger);

    // Coordinate transformations
    Vector2 ScreenToVirtual(Vector2 screenPos) const;
    Vector2 VirtualToScreen(Vector2 virtualPos) const;

    bool IsScreenPointInsideViewport(Vector2 screenPos) const;

    // Accessors
    int GetVirtualWidth() const { return m_virtualWidth; }
    int GetVirtualHeight() const { return m_virtualHeight; }
    int GetWindowWidth() const { return m_windowWidth; }
    int GetWindowHeight() const { return m_windowHeight; }
    float GetScale() const { return m_scale; }
    float GetAspectRatio() const { return m_aspectRatio; }

    Rectangle GetSourceRect() const { return m_sourceRect; }
    Rectangle GetDestRect() const { return m_destRect; }
    Vector2 GetOrigin() const { return m_origin; }
    bool IsLetterboxed() const;

private:
    void Recalculate();

    int m_virtualWidth = 640;
    int m_virtualHeight = 360;
    int m_windowWidth = 1280;
    int m_windowHeight = 720;
    float m_aspectRatio = 16.0f / 9.0f;
    float m_scale = 2.0f;
    bool m_preferIntegerScaling = false;

    Rectangle m_sourceRect{ 0.0f, 0.0f, 640.0f, -360.0f };
    Rectangle m_destRect{ 0.0f, 0.0f, 1280.0f, 720.0f };
    Vector2 m_origin{ 0.0f, 0.0f };
};

} // namespace wildbound::graphics


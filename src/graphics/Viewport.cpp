#include "Viewport.hpp"
#include <algorithm>
#include <cmath>

namespace wildbound::graphics {

Viewport::Viewport(int virtualWidth, int virtualHeight, int windowWidth, int windowHeight, bool preferIntegerScaling)
    : m_virtualWidth(virtualWidth > 0 ? virtualWidth : 640)
    , m_virtualHeight(virtualHeight > 0 ? virtualHeight : 360)
    , m_windowWidth(windowWidth > 0 ? windowWidth : 1280)
    , m_windowHeight(windowHeight > 0 ? windowHeight : 720)
    , m_preferIntegerScaling(preferIntegerScaling)
{
    Recalculate();
}

void Viewport::Update(int newWindowWidth, int newWindowHeight) {
    if (newWindowWidth > 0 && newWindowHeight > 0) {
        m_windowWidth = newWindowWidth;
        m_windowHeight = newWindowHeight;
        Recalculate();
    }
}

void Viewport::SetVirtualResolution(int virtualWidth, int virtualHeight) {
    if (virtualWidth > 0 && virtualHeight > 0) {
        m_virtualWidth = virtualWidth;
        m_virtualHeight = virtualHeight;
        Recalculate();
    }
}

void Viewport::SetPreferIntegerScaling(bool preferInteger) {
    m_preferIntegerScaling = preferInteger;
    Recalculate();
}

void Viewport::Recalculate() {
    m_aspectRatio = static_cast<float>(m_virtualWidth) / static_cast<float>(m_virtualHeight);

    float scaleX = static_cast<float>(m_windowWidth) / static_cast<float>(m_virtualWidth);
    float scaleY = static_cast<float>(m_windowHeight) / static_cast<float>(m_virtualHeight);
    float uniformScale = std::min(scaleX, scaleY);

    if (m_preferIntegerScaling) {
        float floorScale = std::floor(uniformScale);
        m_scale = (floorScale >= 1.0f) ? floorScale : uniformScale;
    } else {
        m_scale = uniformScale;
    }

    if (m_scale <= 0.0f) {
        m_scale = 1.0f;
    }

    float scaledWidth = static_cast<float>(m_virtualWidth) * m_scale;
    float scaledHeight = static_cast<float>(m_virtualHeight) * m_scale;

    m_destRect.x = std::round((static_cast<float>(m_windowWidth) - scaledWidth) * 0.5f);
    m_destRect.y = std::round((static_cast<float>(m_windowHeight) - scaledHeight) * 0.5f);
    m_destRect.width = scaledWidth;
    m_destRect.height = scaledHeight;

    // In raylib, render texture OpenGL coordinates have an inverted Y axis
    m_sourceRect.x = 0.0f;
    m_sourceRect.y = 0.0f;
    m_sourceRect.width = static_cast<float>(m_virtualWidth);
    m_sourceRect.height = -static_cast<float>(m_virtualHeight);
}

Vector2 Viewport::ScreenToVirtual(Vector2 screenPos) const {
    if (m_destRect.width <= 0.0f || m_destRect.height <= 0.0f) {
        return Vector2{ 0.0f, 0.0f };
    }

    Vector2 virtualPos;
    virtualPos.x = (screenPos.x - m_destRect.x) * (static_cast<float>(m_virtualWidth) / m_destRect.width);
    virtualPos.y = (screenPos.y - m_destRect.y) * (static_cast<float>(m_virtualHeight) / m_destRect.height);
    return virtualPos;
}

Vector2 Viewport::VirtualToScreen(Vector2 virtualPos) const {
    if (m_virtualWidth <= 0 || m_virtualHeight <= 0) {
        return Vector2{ 0.0f, 0.0f };
    }

    Vector2 screenPos;
    screenPos.x = m_destRect.x + virtualPos.x * (m_destRect.width / static_cast<float>(m_virtualWidth));
    screenPos.y = m_destRect.y + virtualPos.y * (m_destRect.height / static_cast<float>(m_virtualHeight));
    return screenPos;
}

bool Viewport::IsScreenPointInsideViewport(Vector2 screenPos) const {
    return (screenPos.x >= m_destRect.x &&
            screenPos.x <= m_destRect.x + m_destRect.width &&
            screenPos.y >= m_destRect.y &&
            screenPos.y <= m_destRect.y + m_destRect.height);
}

bool Viewport::IsLetterboxed() const {
    return (m_destRect.x > 0.0f || m_destRect.y > 0.0f ||
            m_destRect.width < static_cast<float>(m_windowWidth) ||
            m_destRect.height < static_cast<float>(m_windowHeight));
}

} // namespace wildbound::graphics


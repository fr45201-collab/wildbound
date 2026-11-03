#include "Camera2D.hpp"
#include <algorithm>
#include <cmath>

namespace wildbound::graphics {

Camera2DSystem::Camera2DSystem(int virtualWidth, int virtualHeight)
    : m_virtualWidth(virtualWidth > 0 ? virtualWidth : 640)
    , m_virtualHeight(virtualHeight > 0 ? virtualHeight : 360)
{
    Reset();
}

void Camera2DSystem::Reset() {
    m_camera.target = Vector2{ 0.0f, 0.0f };
    m_camera.offset = Vector2{
        static_cast<float>(m_virtualWidth) * 0.5f,
        static_cast<float>(m_virtualHeight) * 0.5f
    };
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;

    m_desiredTarget = m_camera.target;
    m_followedObject = nullptr;
    m_mode = CameraMode::Free;
}

void Camera2DSystem::SetVirtualDimensions(int virtualWidth, int virtualHeight) {
    if (virtualWidth > 0 && virtualHeight > 0) {
        m_virtualWidth = virtualWidth;
        m_virtualHeight = virtualHeight;
        m_camera.offset = Vector2{
            static_cast<float>(m_virtualWidth) * 0.5f,
            static_cast<float>(m_virtualHeight) * 0.5f
        };
        ClampToBounds();
    }
}

void Camera2DSystem::SetTarget(Vector2 target) {
    m_camera.target = target;
    m_desiredTarget = target;
    ClampToBounds();
}

void Camera2DSystem::SetOffset(Vector2 offset) {
    m_camera.offset = offset;
}

void Camera2DSystem::SetPosition(Vector2 position) {
    SetTarget(position);
}

void Camera2DSystem::Move(Vector2 delta) {
    m_camera.target.x += delta.x;
    m_camera.target.y += delta.y;
    m_desiredTarget = m_camera.target;
    ClampToBounds();
}

void Camera2DSystem::SetZoom(float zoom) {
    m_camera.zoom = std::clamp(zoom, m_minZoom, m_maxZoom);
    ClampToBounds();
}

void Camera2DSystem::ZoomBy(float deltaZoom) {
    SetZoom(m_camera.zoom + deltaZoom);
}

void Camera2DSystem::SetRotation(float degrees) {
    m_camera.rotation = degrees;
}

void Camera2DSystem::SetZoomLimits(float minZoom, float maxZoom) {
    if (minZoom > 0.0f && maxZoom >= minZoom) {
        m_minZoom = minZoom;
        m_maxZoom = maxZoom;
        m_camera.zoom = std::clamp(m_camera.zoom, m_minZoom, m_maxZoom);
        ClampToBounds();
    }
}

void Camera2DSystem::SetFollowTarget(const Vector2* targetPtr) {
    m_followedObject = targetPtr;
    if (m_followedObject) {
        m_desiredTarget = *m_followedObject;
        m_mode = CameraMode::Follow;
    }
}

void Camera2DSystem::SetFollowTarget(Vector2 targetPos) {
    m_desiredTarget = targetPos;
    m_followedObject = nullptr;
    m_mode = CameraMode::Follow;
}

void Camera2DSystem::SetWorldBounds(Rectangle bounds) {
    m_worldBounds = bounds;
    m_hasWorldBounds = (bounds.width > 0.0f && bounds.height > 0.0f);
    ClampToBounds();
}

void Camera2DSystem::Update(float dt) {
    if (m_mode == CameraMode::Follow) {
        if (m_followedObject != nullptr) {
            m_desiredTarget = *m_followedObject;
        }

        // Exponential smoothing: frame-rate independent spring interpolation
        if (m_followSpeed > 0.0f && dt > 0.0f) {
            float t = 1.0f - std::exp(-m_followSpeed * dt);
            t = std::clamp(t, 0.0f, 1.0f);
            m_camera.target.x += (m_desiredTarget.x - m_camera.target.x) * t;
            m_camera.target.y += (m_desiredTarget.y - m_camera.target.y) * t;
        } else {
            m_camera.target = m_desiredTarget;
        }
    }

    ClampToBounds();
}

void Camera2DSystem::ClampToBounds() {
    if (!m_hasWorldBounds || m_camera.zoom <= 0.0f) {
        return;
    }

    // Visible world dimensions given current virtual resolution and camera zoom
    float visibleWidth = static_cast<float>(m_virtualWidth) / m_camera.zoom;
    float visibleHeight = static_cast<float>(m_virtualHeight) / m_camera.zoom;

    // Horizontal clamping:
    // If world is larger than viewport: clamp camera within bounds.
    // If world is smaller than viewport: center the world inside the viewport.
    if (m_worldBounds.width >= visibleWidth) {
        float minX = m_worldBounds.x + visibleWidth * 0.5f;
        float maxX = m_worldBounds.x + m_worldBounds.width - visibleWidth * 0.5f;
        m_camera.target.x = std::clamp(m_camera.target.x, minX, maxX);
    } else {
        m_camera.target.x = m_worldBounds.x + m_worldBounds.width * 0.5f;
    }

    // Vertical clamping:
    if (m_worldBounds.height >= visibleHeight) {
        float minY = m_worldBounds.y + visibleHeight * 0.5f;
        float maxY = m_worldBounds.y + m_worldBounds.height - visibleHeight * 0.5f;
        m_camera.target.y = std::clamp(m_camera.target.y, minY, maxY);
    } else {
        m_camera.target.y = m_worldBounds.y + m_worldBounds.height * 0.5f;
    }
}

Vector2 Camera2DSystem::WorldToVirtual(Vector2 worldPos) const {
    return GetWorldToScreen2D(worldPos, m_camera);
}

Vector2 Camera2DSystem::VirtualToWorld(Vector2 virtualPos) const {
    return GetScreenToWorld2D(virtualPos, m_camera);
}

Vector2 Camera2DSystem::WorldToScreen(Vector2 worldPos, const Viewport& viewport) const {
    Vector2 virtualPos = WorldToVirtual(worldPos);
    return viewport.VirtualToScreen(virtualPos);
}

Vector2 Camera2DSystem::ScreenToWorld(Vector2 screenPos, const Viewport& viewport) const {
    Vector2 virtualPos = viewport.ScreenToVirtual(screenPos);
    return VirtualToWorld(virtualPos);
}

Camera2D Camera2DSystem::GetRawCamera() const {
    return m_camera;
}

Camera2D Camera2DSystem::GetRenderCamera() const {
    if (!m_pixelSnap) {
        return m_camera;
    }

    // Snapping camera target & offset to integer pixel coordinates
    // prevents sub-pixel blurring and texture jitter in pixel art.
    Camera2D snapped = m_camera;
    snapped.target.x = std::round(m_camera.target.x);
    snapped.target.y = std::round(m_camera.target.y);
    snapped.offset.x = std::round(m_camera.offset.x);
    snapped.offset.y = std::round(m_camera.offset.y);
    return snapped;
}

void Camera2DSystem::Begin() {
    BeginMode2D(GetRenderCamera());
}

void Camera2DSystem::End() {
    EndMode2D();
}

} // namespace wildbound::graphics


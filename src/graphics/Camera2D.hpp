#pragma once

#include "Viewport.hpp"
#include <raylib.h>

namespace wildbound::graphics {

enum class CameraMode {
    Free = 0,
    Follow,
    Fixed
};

/**
 * Robust 2D Camera abstraction wrapping raylib Camera2D.
 * Supports smooth following, world boundary clamping, zoom constraints,
 * pixel-grid snapping, and multi-space coordinate transformations.
 */
class Camera2DSystem {
public:
    Camera2DSystem(int virtualWidth = 640, int virtualHeight = 360);

    void Update(float dt);

    // Camera control
    void SetTarget(Vector2 target);
    Vector2 GetTarget() const { return m_camera.target; }

    void SetOffset(Vector2 offset);
    Vector2 GetOffset() const { return m_camera.offset; }

    void SetPosition(Vector2 position);
    void Move(Vector2 delta);

    void SetZoom(float zoom);
    void ZoomBy(float deltaZoom);
    float GetZoom() const { return m_camera.zoom; }

    void SetRotation(float degrees);
    float GetRotation() const { return m_camera.rotation; }

    // Follow mechanism
    void SetFollowTarget(const Vector2* targetPtr);
    void SetFollowTarget(Vector2 targetPos);
    void SetFollowSpeed(float speed) { m_followSpeed = speed; }
    float GetFollowSpeed() const { return m_followSpeed; }

    // Mode
    void SetMode(CameraMode mode) { m_mode = mode; }
    CameraMode GetMode() const { return m_mode; }

    // World Boundaries
    void SetWorldBounds(Rectangle bounds);
    Rectangle GetWorldBounds() const { return m_worldBounds; }
    bool HasWorldBounds() const { return m_hasWorldBounds; }
    void ClearWorldBounds() { m_hasWorldBounds = false; }

    // Viewport & Pixel-art configuration
    void SetVirtualDimensions(int virtualWidth, int virtualHeight);
    void SetPixelSnapping(bool enabled) { m_pixelSnap = enabled; }
    bool IsPixelSnapping() const { return m_pixelSnap; }

    // Zoom limits
    void SetZoomLimits(float minZoom, float maxZoom);
    float GetMinZoom() const { return m_minZoom; }
    float GetMaxZoom() const { return m_maxZoom; }

    void Reset();

    // Coordinate conversion functions
    Vector2 WorldToVirtual(Vector2 worldPos) const;
    Vector2 VirtualToWorld(Vector2 virtualPos) const;
    Vector2 WorldToScreen(Vector2 worldPos, const Viewport& viewport) const;
    Vector2 ScreenToWorld(Vector2 screenPos, const Viewport& viewport) const;

    // Raylib camera integration
    void Begin();
    void End();
    Camera2D GetRawCamera() const;
    Camera2D GetRenderCamera() const;

private:
    void ClampToBounds();

    Camera2D m_camera{};
    CameraMode m_mode = CameraMode::Free;

    Vector2 m_desiredTarget{ 0.0f, 0.0f };
    const Vector2* m_followedObject = nullptr;
    float m_followSpeed = 5.0f; // Interpolation speed in Hz

    int m_virtualWidth = 640;
    int m_virtualHeight = 360;

    Rectangle m_worldBounds{ 0.0f, 0.0f, 0.0f, 0.0f };
    bool m_hasWorldBounds = false;

    float m_minZoom = 0.5f;
    float m_maxZoom = 4.0f;
    bool m_pixelSnap = true;
};

} // namespace wildbound::graphics


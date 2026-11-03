#pragma once

#include <raylib.h>

namespace wildbound::entities {

/**
 * Centralized input handler and motion vector calculator for character movement.
 * Supports WASD, arrow keys, running modifiers, and test injection.
 */
class CharacterController {
public:
    CharacterController() = default;

    void Update();

    // Motion outputs
    Vector2 GetMovementDirection() const { return m_movementDirection; }
    bool IsMoving() const { return m_isMoving; }
    bool IsRunning() const { return m_isRunning; }
    bool IsPauseRequested() const { return m_pauseRequested; }

    // Manual input override (for automated testing / scripting)
    void SetManualOverride(bool enabled) { m_manualOverride = enabled; }
    bool IsManualOverride() const { return m_manualOverride; }
    void SetManualInput(Vector2 direction, bool isRunning);

    // Configuration
    void SetNormalizeDiagonal(bool normalize) { m_normalizeDiagonal = normalize; }
    bool GetNormalizeDiagonal() const { return m_normalizeDiagonal; }

private:
    Vector2 m_movementDirection{ 0.0f, 0.0f };
    bool m_isMoving = false;
    bool m_isRunning = false;
    bool m_pauseRequested = false;

    bool m_normalizeDiagonal = true;
    bool m_manualOverride = false;
};

} // namespace wildbound::entities


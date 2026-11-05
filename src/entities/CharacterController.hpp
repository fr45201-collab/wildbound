#pragma once

#include <raylib.h>

namespace wildbound::input {
    class InputManager;
}

namespace wildbound::entities {

/**
 * Centralized input handler and motion vector calculator for character movement.
 * Supports logical InputManager actions, WASD/arrows fallback, and test injection.
 */
class CharacterController {
public:
    CharacterController() = default;

    void Update(const input::InputManager* inputMgr = nullptr);

    void SetInputManager(const input::InputManager* inputMgr) { m_inputManager = inputMgr; }
    const input::InputManager* GetInputManager() const { return m_inputManager; }

    // Motion outputs
    Vector2 GetMovementDirection() const { return m_movementDirection; }
    bool IsMoving() const { return m_isMoving; }
    bool IsRunning() const { return m_isRunning; }
    bool IsAttackRequested() const { return m_attackRequested; }
    bool IsPauseRequested() const { return m_pauseRequested; }

    // Manual input override (for automated testing / scripting)
    void SetManualOverride(bool enabled) { m_manualOverride = enabled; }
    bool IsManualOverride() const { return m_manualOverride; }
    void SetManualInput(Vector2 direction, bool isRunning);
    void SetManualAttack(bool attack) { m_attackRequested = attack; }

    // Configuration
    void SetNormalizeDiagonal(bool normalize) { m_normalizeDiagonal = normalize; }
    bool GetNormalizeDiagonal() const { return m_normalizeDiagonal; }

private:
    const input::InputManager* m_inputManager = nullptr;
    Vector2 m_movementDirection{ 0.0f, 0.0f };
    bool m_isMoving = false;
    bool m_isRunning = false;
    bool m_attackRequested = false;
    bool m_pauseRequested = false;

    bool m_normalizeDiagonal = true;
    bool m_manualOverride = false;
};

} // namespace wildbound::entities


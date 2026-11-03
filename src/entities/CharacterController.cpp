#include "CharacterController.hpp"
#include <cmath>

namespace wildbound::entities {

void CharacterController::SetManualInput(Vector2 direction, bool isRunning) {
    m_manualOverride = true;
    m_isRunning = isRunning;

    float lenSq = (direction.x * direction.x) + (direction.y * direction.y);
    if (lenSq > 0.0001f) {
        m_isMoving = true;
        if (m_normalizeDiagonal && lenSq > 1.0001f) {
            float invLen = 1.0f / std::sqrt(lenSq);
            m_movementDirection = Vector2{ direction.x * invLen, direction.y * invLen };
        } else {
            m_movementDirection = direction;
        }
    } else {
        m_isMoving = false;
        m_movementDirection = Vector2{ 0.0f, 0.0f };
    }
}

void CharacterController::Update() {
    if (m_manualOverride) {
        return;
    }

    Vector2 dir{ 0.0f, 0.0f };

    // 4-Directional Input: WASD and Arrow Keys
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    dir.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  dir.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  dir.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dir.x += 1.0f;

    // Running modifier: Left Shift or Right Shift
    m_isRunning = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));

    // Pause request: ESC
    m_pauseRequested = IsKeyPressed(KEY_ESCAPE);

    float lenSq = (dir.x * dir.x) + (dir.y * dir.y);
    if (lenSq > 0.0001f) {
        m_isMoving = true;
        if (m_normalizeDiagonal && lenSq > 1.0001f) {
            float invLen = 1.0f / std::sqrt(lenSq);
            m_movementDirection = Vector2{ dir.x * invLen, dir.y * invLen };
        } else {
            m_movementDirection = dir;
        }
    } else {
        m_isMoving = false;
        m_movementDirection = Vector2{ 0.0f, 0.0f };
    }
}

} // namespace wildbound::entities


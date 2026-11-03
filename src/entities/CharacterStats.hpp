#pragma once

namespace wildbound::entities {

struct CharacterStats {
    float walkSpeed = 100.0f;       // Pixels / second
    float runSpeed = 165.0f;        // Pixels / second
    bool normalizeDiagonal = true;  // Prevent sqrt(2) diagonal speed boost
    float acceleration = 0.0f;      // 0.0 = immediate response

    float GetCurrentSpeed(bool isRunning) const {
        return isRunning ? runSpeed : walkSpeed;
    }
};

} // namespace wildbound::entities


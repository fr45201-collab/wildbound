#include "GamepadInput.hpp"
#include <algorithm>
#include <cmath>

namespace wildbound::input {

Vector2 GamepadInput::ProcessStickDeadzone(Vector2 raw) const noexcept {
    float mag = std::sqrt(raw.x * raw.x + raw.y * raw.y);
    if (mag <= m_deadzone) {
        return Vector2{ 0.0f, 0.0f };
    }

    // Radial normalization remapped smoothly from deadzone to 1.0
    float normalizedMag = (mag - m_deadzone) / (1.0f - m_deadzone);
    normalizedMag = std::clamp(normalizedMag, 0.0f, 1.0f);

    float invMag = 1.0f / mag;
    return Vector2{ raw.x * invMag * normalizedMag, raw.y * invMag * normalizedMag };
}

void GamepadInput::Update(int gamepadIndex) {
    if (gamepadIndex < 0 || gamepadIndex >= 4) return;

    auto& state = m_controllers[gamepadIndex];

    if (m_simulated) {
        // In simulated mode, compute pressed/released from simulated down states
        for (int b = 0; b < 32; ++b) {
            bool down = state.buttonsDown[b];
            bool prev = m_prevButtonsDown[gamepadIndex][b];
            state.buttonsPressed[b] = (down && !prev);
            state.buttonsReleased[b] = (!down && prev);
            m_prevButtonsDown[gamepadIndex][b] = down;
        }
        return;
    }

    bool available = ::IsGamepadAvailable(gamepadIndex);
    state.connected = available;

    if (!available) {
        state.name = "Disconnected";
        state.leftStick = Vector2{ 0.0f, 0.0f };
        state.rightStick = Vector2{ 0.0f, 0.0f };
        state.leftTrigger = 0.0f;
        state.rightTrigger = 0.0f;
        state.buttonsDown.fill(false);
        state.buttonsPressed.fill(false);
        state.buttonsReleased.fill(false);
        m_prevButtonsDown[gamepadIndex].fill(false);
        return;
    }

    const char* gName = ::GetGamepadName(gamepadIndex);
    state.name = (gName != nullptr) ? gName : "Gamepad";

    // Read analog sticks
    Vector2 rawLS{
        ::GetGamepadAxisMovement(gamepadIndex, GAMEPAD_AXIS_LEFT_X),
        ::GetGamepadAxisMovement(gamepadIndex, GAMEPAD_AXIS_LEFT_Y)
    };
    state.leftStick = ProcessStickDeadzone(rawLS);

    Vector2 rawRS{
        ::GetGamepadAxisMovement(gamepadIndex, GAMEPAD_AXIS_RIGHT_X),
        ::GetGamepadAxisMovement(gamepadIndex, GAMEPAD_AXIS_RIGHT_Y)
    };
    state.rightStick = ProcessStickDeadzone(rawRS);

    // Read triggers: raylib trigger axis returns -1.0 to 1.0, or 0.0 to 1.0 depending on platform
    float rawLT = ::GetGamepadAxisMovement(gamepadIndex, GAMEPAD_AXIS_LEFT_TRIGGER);
    float rawRT = ::GetGamepadAxisMovement(gamepadIndex, GAMEPAD_AXIS_RIGHT_TRIGGER);
    // Normalize to 0.0 - 1.0
    state.leftTrigger = (rawLT > 0.0f) ? rawLT : 0.0f;
    state.rightTrigger = (rawRT > 0.0f) ? rawRT : 0.0f;

    // Read buttons (Standard raylib buttons 0 through 18)
    for (int b = 0; b < 32; ++b) {
        bool down = ::IsGamepadButtonDown(gamepadIndex, b);
        bool pressed = ::IsGamepadButtonPressed(gamepadIndex, b);
        bool released = ::IsGamepadButtonReleased(gamepadIndex, b);

        state.buttonsDown[b] = down;
        state.buttonsPressed[b] = pressed;
        state.buttonsReleased[b] = released;
        m_prevButtonsDown[gamepadIndex][b] = down;
    }
}

bool GamepadInput::IsConnected(int gamepadIndex) const noexcept {
    if (gamepadIndex < 0 || gamepadIndex >= 4) return false;
    return m_controllers[gamepadIndex].connected;
}

std::string GamepadInput::GetName(int gamepadIndex) const {
    if (gamepadIndex < 0 || gamepadIndex >= 4) return "Unknown";
    return m_controllers[gamepadIndex].name;
}

Vector2 GamepadInput::GetLeftStick(int gamepadIndex) const noexcept {
    if (gamepadIndex < 0 || gamepadIndex >= 4) return Vector2{ 0.0f, 0.0f };
    return m_controllers[gamepadIndex].leftStick;
}

Vector2 GamepadInput::GetRightStick(int gamepadIndex) const noexcept {
    if (gamepadIndex < 0 || gamepadIndex >= 4) return Vector2{ 0.0f, 0.0f };
    return m_controllers[gamepadIndex].rightStick;
}

float GamepadInput::GetLeftTrigger(int gamepadIndex) const noexcept {
    if (gamepadIndex < 0 || gamepadIndex >= 4) return 0.0f;
    return m_controllers[gamepadIndex].leftTrigger;
}

float GamepadInput::GetRightTrigger(int gamepadIndex) const noexcept {
    if (gamepadIndex < 0 || gamepadIndex >= 4) return 0.0f;
    return m_controllers[gamepadIndex].rightTrigger;
}

bool GamepadInput::IsButtonDown(int button, int gamepadIndex) const noexcept {
    if (gamepadIndex < 0 || gamepadIndex >= 4 || button < 0 || button >= 32) return false;
    return m_controllers[gamepadIndex].buttonsDown[button];
}

bool GamepadInput::IsButtonPressed(int button, int gamepadIndex) const noexcept {
    if (gamepadIndex < 0 || gamepadIndex >= 4 || button < 0 || button >= 32) return false;
    return m_controllers[gamepadIndex].buttonsPressed[button];
}

bool GamepadInput::IsButtonReleased(int button, int gamepadIndex) const noexcept {
    if (gamepadIndex < 0 || gamepadIndex >= 4 || button < 0 || button >= 32) return false;
    return m_controllers[gamepadIndex].buttonsReleased[button];
}

void GamepadInput::SimulateConnection(int gamepadIndex, bool connected, const std::string& name) {
    if (gamepadIndex < 0 || gamepadIndex >= 4) return;
    m_simulated = true;
    m_controllers[gamepadIndex].connected = connected;
    m_controllers[gamepadIndex].name = name;
    if (!connected) {
        m_controllers[gamepadIndex].leftStick = Vector2{ 0.0f, 0.0f };
        m_controllers[gamepadIndex].rightStick = Vector2{ 0.0f, 0.0f };
        m_controllers[gamepadIndex].leftTrigger = 0.0f;
        m_controllers[gamepadIndex].rightTrigger = 0.0f;
        m_controllers[gamepadIndex].buttonsDown.fill(false);
        m_controllers[gamepadIndex].buttonsPressed.fill(false);
        m_controllers[gamepadIndex].buttonsReleased.fill(false);
    }
}

void GamepadInput::SimulateStick(int gamepadIndex, bool leftStick, Vector2 rawValues) {
    if (gamepadIndex < 0 || gamepadIndex >= 4) return;
    m_simulated = true;
    Vector2 processed = ProcessStickDeadzone(rawValues);
    if (leftStick) {
        m_controllers[gamepadIndex].leftStick = processed;
    } else {
        m_controllers[gamepadIndex].rightStick = processed;
    }
}

void GamepadInput::SimulateTrigger(int gamepadIndex, bool leftTrigger, float value) {
    if (gamepadIndex < 0 || gamepadIndex >= 4) return;
    m_simulated = true;
    float clamped = std::clamp(value, 0.0f, 1.0f);
    if (leftTrigger) {
        m_controllers[gamepadIndex].leftTrigger = clamped;
    } else {
        m_controllers[gamepadIndex].rightTrigger = clamped;
    }
}

void GamepadInput::SimulateButton(int gamepadIndex, int button, bool isDown) {
    if (gamepadIndex < 0 || gamepadIndex >= 4 || button < 0 || button >= 32) return;
    m_simulated = true;
    m_controllers[gamepadIndex].buttonsDown[button] = isDown;
}

} // namespace wildbound::input


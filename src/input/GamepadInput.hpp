#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

namespace wildbound::input {

struct GamepadState {
    bool connected = false;
    std::string name = "Disconnected";
    Vector2 leftStick{ 0.0f, 0.0f };
    Vector2 rightStick{ 0.0f, 0.0f };
    float leftTrigger = 0.0f;
    float rightTrigger = 0.0f;
    std::array<bool, 32> buttonsDown{};
    std::array<bool, 32> buttonsPressed{};
    std::array<bool, 32> buttonsReleased{};
};

/**
 * Encapsulates gamepad hardware polling and deterministic simulated input.
 */
class GamepadInput {
public:
    GamepadInput() = default;

    void SetDeadzone(float deadzone) noexcept {
        m_deadzone = std::clamp(deadzone, 0.0f, 0.9f);
    }
    float GetDeadzone() const noexcept { return m_deadzone; }

    void SetTriggerThreshold(float threshold) noexcept {
        m_triggerThreshold = std::clamp(threshold, 0.05f, 0.95f);
    }
    float GetTriggerThreshold() const noexcept { return m_triggerThreshold; }

    /**
     * Polls raylib gamepad state or processes simulated state.
     */
    void Update(int gamepadIndex = 0);

    bool IsConnected(int gamepadIndex = 0) const noexcept;
    std::string GetName(int gamepadIndex = 0) const;

    Vector2 GetLeftStick(int gamepadIndex = 0) const noexcept;
    Vector2 GetRightStick(int gamepadIndex = 0) const noexcept;
    float GetLeftTrigger(int gamepadIndex = 0) const noexcept;
    float GetRightTrigger(int gamepadIndex = 0) const noexcept;

    bool IsButtonDown(int button, int gamepadIndex = 0) const noexcept;
    bool IsButtonPressed(int button, int gamepadIndex = 0) const noexcept;
    bool IsButtonReleased(int button, int gamepadIndex = 0) const noexcept;

    // Simulated test adapter injection (for automated headless test suites)
    void SetSimulated(bool enabled) noexcept { m_simulated = enabled; }
    bool IsSimulated() const noexcept { return m_simulated; }
    void SimulateConnection(int gamepadIndex, bool connected, const std::string& name = "Virtual Controller");
    void SimulateStick(int gamepadIndex, bool leftStick, Vector2 rawValues);
    void SimulateTrigger(int gamepadIndex, bool leftTrigger, float value);
    void SimulateButton(int gamepadIndex, int button, bool isDown);

    /**
     * Applies radial deadzone and normalizes vector.
     */
    Vector2 ProcessStickDeadzone(Vector2 raw) const noexcept;

private:
    float m_deadzone = 0.20f;             // 20% radial deadzone
    float m_triggerThreshold = 0.50f;     // 50% trigger actuation threshold
    bool m_simulated = false;

    // Up to 4 controllers tracked
    std::array<GamepadState, 4> m_controllers{};
    std::array<std::array<bool, 32>, 4> m_prevButtonsDown{};
};

} // namespace wildbound::input

#pragma once

#include "InputAction.hpp"
#include "InputBinding.hpp"
#include "InputContext.hpp"
#include "InputBuffer.hpp"
#include "GamepadInput.hpp"
#include "InputSettings.hpp"
#include <raylib.h>
#include <array>
#include <string>
#include <vector>

namespace wildbound::graphics {
    class Viewport;
    class Camera2DSystem;
}

namespace wildbound::input {

struct ActionState {
    bool pressed = false;
    bool held = false;
    bool released = false;
    float value = 0.0f;
};

/**
 * Centralized, authoritative Input Management System for Wildbound.
 * Translates hardware signals (Keyboard, Mouse, Gamepad) into logical InputActions,
 * regulates input contexts, manages action buffering, and calculates mouse coordinates.
 */
class InputManager {
public:
    InputManager();
    ~InputManager() = default;

    /**
     * Central per-frame update.
     * Polls hardware devices, computes coordinate conversions, updates buffers,
     * processes active contexts, and evaluates logical action states.
     */
    void Update(float dt,
                const graphics::Viewport* viewport = nullptr,
                const graphics::Camera2DSystem* camera = nullptr);

    // Logical action queries
    bool WasActionPressed(InputAction action) const noexcept;
    bool IsActionHeld(InputAction action) const noexcept;
    bool WasActionReleased(InputAction action) const noexcept;
    float GetActionValue(InputAction action) const noexcept;

    /**
     * Consumes a buffered action if available or checks instantaneous press.
     * Guaranteed safe for single-frame consumption.
     */
    bool ConsumeAction(InputAction action) noexcept;
    bool HasBufferedAction(InputAction action) const noexcept;

    /**
     * Calculates composite 2D movement vector from digital keys or analog gamepad stick.
     * Diagonal movement is optionally normalized.
     */
    Vector2 GetMovementVector(bool normalize = true) const noexcept;

    // Mouse coordinates
    Vector2 GetMouseScreenPosition() const noexcept { return m_mouseScreen; }
    Vector2 GetMouseVirtualPosition() const noexcept { return m_mouseVirtual; }
    Vector2 GetMouseWorldPosition() const noexcept { return m_mouseWorld; }
    float GetMouseWheelMove() const noexcept { return m_mouseWheelMove; }
    Vector2 GetMouseAimDirection(Vector2 originWorld) const noexcept;

    // Context management
    InputContextManager& GetContextManager() noexcept { return m_contextManager; }
    const InputContextManager& GetContextManager() const noexcept { return m_contextManager; }
    void PushContext(InputContextType type);
    void PopContext();
    void SetContext(InputContextType type);

    // Buffering
    InputBuffer& GetBuffer() noexcept { return m_buffer; }
    const InputBuffer& GetBuffer() const noexcept { return m_buffer; }

    // Gamepad
    GamepadInput& GetGamepad() noexcept { return m_gamepad; }
    const GamepadInput& GetGamepad() const noexcept { return m_gamepad; }

    // Settings & Rebinding
    InputSettings& GetSettings() noexcept { return m_settings; }
    const InputSettings& GetSettings() const noexcept { return m_settings; }

    // Active device tracking
    DeviceType GetActiveDevice() const noexcept { return m_activeDevice; }
    bool IsGamepadActive() const noexcept { return m_activeDevice == DeviceType::Gamepad; }

    // Safety & Lifecycle
    void Reset() noexcept;

    // Automated testing / injection adapter
    void SetSimulated(bool enabled) noexcept { m_simulated = enabled; }
    bool IsSimulated() const noexcept { return m_simulated; }
    void InjectAction(InputAction action, bool pressed, bool held, bool released, float value = 1.0f);
    void InjectMousePositions(Vector2 screen, Vector2 virtualPos, Vector2 world);
    void ClearInjected();

private:
    void PollHardware(float dt,
                      const graphics::Viewport* viewport,
                      const graphics::Camera2DSystem* camera);
    void ProcessRebinding();
    bool EvaluateBinding(const InputBinding& b, float& outValue) const noexcept;

private:
    InputSettings m_settings;
    InputContextManager m_contextManager;
    InputBuffer m_buffer;
    GamepadInput m_gamepad;

    DeviceType m_activeDevice = DeviceType::Keyboard;
    bool m_simulated = false;

    // Mouse positions
    Vector2 m_mouseScreen{ 0.0f, 0.0f };
    Vector2 m_mouseVirtual{ 0.0f, 0.0f };
    Vector2 m_mouseWorld{ 0.0f, 0.0f };
    float m_mouseWheelMove = 0.0f;

    // Action states
    std::array<ActionState, ACTION_COUNT> m_actionStates{};
    std::array<bool, ACTION_COUNT> m_prevHeld{};
};

} // namespace wildbound::input

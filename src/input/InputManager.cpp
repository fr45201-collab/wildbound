#include "InputManager.hpp"
#include "../graphics/Viewport.hpp"
#include "../graphics/Camera2D.hpp"
#include <cmath>
#include <algorithm>

namespace wildbound::input {

InputManager::InputManager() {
    Reset();
}

void InputManager::Reset() noexcept {
    m_mouseScreen = Vector2{ 0.0f, 0.0f };
    m_mouseVirtual = Vector2{ 0.0f, 0.0f };
    m_mouseWorld = Vector2{ 0.0f, 0.0f };
    m_mouseWheelMove = 0.0f;

    for (auto& st : m_actionStates) {
        st.pressed = false;
        st.held = false;
        st.released = false;
        st.value = 0.0f;
    }
    m_prevHeld.fill(false);
    m_buffer.Clear();
}

void InputManager::PushContext(InputContextType type) {
    m_contextManager.PushContext(type);
    // Clear inappropriate buffered actions
    m_buffer.Clear();
}

void InputManager::PopContext() {
    m_contextManager.PopContext();
    m_buffer.Clear();
}

void InputManager::SetContext(InputContextType type) {
    m_contextManager.SetContext(type);
    m_buffer.Clear();
}

bool InputManager::EvaluateBinding(const InputBinding& b, float& outValue) const noexcept {
    if (!b.IsValid()) return false;

    if (b.device == DeviceType::Keyboard) {
        if (b.source == BindingSourceType::ButtonOrKey) {
            bool down = ::IsKeyDown(b.code);
            if (down) {
                outValue = 1.0f;
                return true;
            }
        }
    } else if (b.device == DeviceType::Mouse) {
        if (b.source == BindingSourceType::ButtonOrKey) {
            bool down = ::IsMouseButtonDown(b.code);
            if (down) {
                outValue = 1.0f;
                return true;
            }
        } else if (b.source == BindingSourceType::Axis) {
            float wheel = ::GetMouseWheelMove();
            if ((b.axisDirection > 0 && wheel > 0.1f) ||
                (b.axisDirection < 0 && wheel < -0.1f)) {
                outValue = std::abs(wheel);
                return true;
            }
        }
    } else if (b.device == DeviceType::Gamepad) {
        if (b.source == BindingSourceType::ButtonOrKey) {
            bool down = m_gamepad.IsButtonDown(b.code);
            if (down) {
                outValue = 1.0f;
                return true;
            }
        } else if (b.source == BindingSourceType::Axis) {
            if (b.axis == GAMEPAD_AXIS_LEFT_X) {
                float val = m_gamepad.GetLeftStick().x;
                if ((b.axisDirection > 0 && val >= b.axisThreshold) ||
                    (b.axisDirection < 0 && val <= -b.axisThreshold)) {
                    outValue = std::abs(val);
                    return true;
                }
            } else if (b.axis == GAMEPAD_AXIS_LEFT_Y) {
                float val = m_gamepad.GetLeftStick().y;
                if ((b.axisDirection > 0 && val >= b.axisThreshold) ||
                    (b.axisDirection < 0 && val <= -b.axisThreshold)) {
                    outValue = std::abs(val);
                    return true;
                }
            } else if (b.axis == GAMEPAD_AXIS_RIGHT_TRIGGER) {
                float val = m_gamepad.GetRightTrigger();
                if (val >= b.axisThreshold) {
                    outValue = val;
                    return true;
                }
            } else if (b.axis == GAMEPAD_AXIS_LEFT_TRIGGER) {
                float val = m_gamepad.GetLeftTrigger();
                if (val >= b.axisThreshold) {
                    outValue = val;
                    return true;
                }
            }
        }
    }

    return false;
}

void InputManager::ProcessRebinding() {
    if (!m_settings.IsRebinding()) return;

    DeviceType targetDev = m_settings.GetRebindingDevice();

    if (targetDev == DeviceType::Keyboard) {
        // Cancel rebinding on Escape
        if (::IsKeyPressed(KEY_ESCAPE)) {
            m_settings.CancelRebinding();
            return;
        }

        // Detect any pressed key
        int pressedKey = ::GetKeyPressed();
        while (pressedKey > 0) {
            if (pressedKey != KEY_ESCAPE) {
                InputBinding nb{ DeviceType::Keyboard, BindingSourceType::ButtonOrKey, pressedKey };
                m_settings.ApplyRebinding(nb, true);
                return;
            }
            pressedKey = ::GetKeyPressed();
        }
    } else if (targetDev == DeviceType::Mouse) {
        for (int mb = 0; mb <= 4; ++mb) {
            if (::IsMouseButtonPressed(mb)) {
                InputBinding nb{ DeviceType::Mouse, BindingSourceType::ButtonOrKey, mb };
                m_settings.ApplyRebinding(nb, true);
                return;
            }
        }
    } else if (targetDev == DeviceType::Gamepad) {
        // Detect buttons
        for (int gb = 0; gb < 32; ++gb) {
            if (m_gamepad.IsButtonPressed(gb)) {
                InputBinding nb{ DeviceType::Gamepad, BindingSourceType::ButtonOrKey, gb };
                m_settings.ApplyRebinding(nb, true);
                return;
            }
        }
    }
}

void InputManager::PollHardware(float dt,
                                const graphics::Viewport* viewport,
                                const graphics::Camera2DSystem* camera) {
    if (m_simulated) {
        m_buffer.Update(dt);
        return;
    }

    // Handle window focus loss
    if (!::IsWindowFocused()) {
        Reset();
        return;
    }

    // 1. Mouse coordinates
    m_mouseScreen = ::GetMousePosition();
    m_mouseWheelMove = ::GetMouseWheelMove();

    if (viewport) {
        m_mouseVirtual = viewport->ScreenToVirtual(m_mouseScreen);
    } else {
        m_mouseVirtual = m_mouseScreen;
    }

    if (camera && viewport) {
        m_mouseWorld = camera->ScreenToWorld(m_mouseScreen, *viewport);
    } else {
        m_mouseWorld = m_mouseVirtual;
    }

    // 2. Poll gamepad
    m_gamepad.Update(0);

    // 3. Process active rebinding if engaged
    if (m_settings.IsRebinding()) {
        ProcessRebinding();
        return;
    }

    // 4. Update input buffer timers
    m_buffer.Update(dt);

    // 5. Evaluate all logical actions
    for (size_t i = 0; i < ACTION_COUNT; ++i) {
        auto action = static_cast<InputAction>(i);
        bool allowed = m_contextManager.IsActionAllowed(action);

        if (!allowed) {
            m_actionStates[i].pressed = false;
            m_actionStates[i].held = false;
            m_actionStates[i].released = m_prevHeld[i]; // Emit clean release if transitioning into blocked context
            m_actionStates[i].value = 0.0f;
            m_prevHeld[i] = false;
            continue;
        }

        float val = 0.0f;
        bool isHeld = false;

        // Evaluate primary & alternate keyboard/mouse bindings
        const auto& prim = m_settings.GetPrimaryBinding(action);
        const auto& alt = m_settings.GetAlternateBinding(action);
        if (EvaluateBinding(prim, val)) {
            isHeld = true;
            m_activeDevice = DeviceType::Keyboard;
        } else if (EvaluateBinding(alt, val)) {
            isHeld = true;
            m_activeDevice = DeviceType::Keyboard;
        }

        // Evaluate gamepad binding
        const auto& gp = m_settings.GetGamepadBinding(action);
        float gpVal = 0.0f;
        if (EvaluateBinding(gp, gpVal)) {
            isHeld = true;
            val = std::max(val, gpVal);
            m_activeDevice = DeviceType::Gamepad;
        }

        bool wasHeld = m_prevHeld[i];
        bool wasPressed = (isHeld && !wasHeld);
        bool wasReleased = (!isHeld && wasHeld);

        m_actionStates[i].pressed = wasPressed;
        m_actionStates[i].held = isHeld;
        m_actionStates[i].released = wasReleased;
        m_actionStates[i].value = isHeld ? val : 0.0f;

        m_prevHeld[i] = isHeld;

        // Automatically buffer actions with non-zero buffer durations on press
        if (wasPressed && m_buffer.GetBufferDuration(action) > 0.0f) {
            m_buffer.BufferAction(action, val);
        }
    }
}

void InputManager::Update(float dt,
                          const graphics::Viewport* viewport,
                          const graphics::Camera2DSystem* camera) {
    PollHardware(dt, viewport, camera);
}

bool InputManager::WasActionPressed(InputAction action) const noexcept {
    size_t idx = static_cast<size_t>(action);
    if (idx >= ACTION_COUNT) return false;
    return m_actionStates[idx].pressed;
}

bool InputManager::IsActionHeld(InputAction action) const noexcept {
    size_t idx = static_cast<size_t>(action);
    if (idx >= ACTION_COUNT) return false;
    return m_actionStates[idx].held;
}

bool InputManager::WasActionReleased(InputAction action) const noexcept {
    size_t idx = static_cast<size_t>(action);
    if (idx >= ACTION_COUNT) return false;
    return m_actionStates[idx].released;
}

float InputManager::GetActionValue(InputAction action) const noexcept {
    size_t idx = static_cast<size_t>(action);
    if (idx >= ACTION_COUNT) return 0.0f;
    return m_actionStates[idx].value;
}

bool InputManager::ConsumeAction(InputAction action) noexcept {
    size_t idx = static_cast<size_t>(action);
    if (idx >= ACTION_COUNT) return false;

    // First attempt to consume from the input buffer
    if (m_buffer.ConsumeAction(action)) {
        m_actionStates[idx].pressed = false;
        return true;
    }

    // Fall back to instantaneous press
    if (m_actionStates[idx].pressed) {
        m_actionStates[idx].pressed = false;
        return true;
    }

    return false;
}

bool InputManager::HasBufferedAction(InputAction action) const noexcept {
    return m_buffer.HasBufferedAction(action) || WasActionPressed(action);
}

Vector2 InputManager::GetMovementVector(bool normalize) const noexcept {
    Vector2 dir{ 0.0f, 0.0f };

    // Check if movement is allowed by active context
    if (!m_contextManager.IsActionAllowed(InputAction::MoveUp)) {
        return dir;
    }

    // Digital keys
    if (IsActionHeld(InputAction::MoveUp))    dir.y -= 1.0f;
    if (IsActionHeld(InputAction::MoveDown))  dir.y += 1.0f;
    if (IsActionHeld(InputAction::MoveLeft))  dir.x -= 1.0f;
    if (IsActionHeld(InputAction::MoveRight)) dir.x += 1.0f;

    // Analog gamepad stick blend
    Vector2 gpStick = m_gamepad.GetLeftStick();
    if (std::abs(gpStick.x) > 0.001f || std::abs(gpStick.y) > 0.001f) {
        dir.x += gpStick.x;
        dir.y += gpStick.y;
    }

    float lenSq = dir.x * dir.x + dir.y * dir.y;
    if (lenSq > 0.0001f) {
        if (normalize && lenSq > 1.0001f) {
            float invLen = 1.0f / std::sqrt(lenSq);
            dir.x *= invLen;
            dir.y *= invLen;
        }
    } else {
        dir = Vector2{ 0.0f, 0.0f };
    }

    return dir;
}

Vector2 InputManager::GetMouseAimDirection(Vector2 originWorld) const noexcept {
    Vector2 diff{ m_mouseWorld.x - originWorld.x, m_mouseWorld.y - originWorld.y };
    float lenSq = diff.x * diff.x + diff.y * diff.y;
    if (lenSq > 0.0001f) {
        float invLen = 1.0f / std::sqrt(lenSq);
        return Vector2{ diff.x * invLen, diff.y * invLen };
    }
    return Vector2{ 0.0f, 1.0f }; // Default facing South
}

void InputManager::InjectAction(InputAction action, bool pressed, bool held, bool released, float value) {
    size_t idx = static_cast<size_t>(action);
    if (idx >= ACTION_COUNT) return;
    m_simulated = true;
    m_actionStates[idx].pressed = pressed;
    m_actionStates[idx].held = held;
    m_actionStates[idx].released = released;
    m_actionStates[idx].value = value;
    m_prevHeld[idx] = held;

    if (pressed && m_buffer.GetBufferDuration(action) > 0.0f) {
        m_buffer.BufferAction(action, value);
    }
}

void InputManager::InjectMousePositions(Vector2 screen, Vector2 virtualPos, Vector2 world) {
    m_simulated = true;
    m_mouseScreen = screen;
    m_mouseVirtual = virtualPos;
    m_mouseWorld = world;
}

void InputManager::ClearInjected() {
    Reset();
}

} // namespace wildbound::input


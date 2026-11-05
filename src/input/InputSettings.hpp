#pragma once

#include "InputAction.hpp"
#include "InputBinding.hpp"
#include <vector>
#include <array>
#include <string>
#include <optional>

namespace wildbound::input {

struct ActionBindingPair {
    InputBinding primary;
    InputBinding alternate;
};

/**
 * Manages action mappings, rebinding workflow, conflict detection, and JSON persistence.
 */
class InputSettings {
public:
    InputSettings();

    /**
     * Resets all bindings to factory defaults.
     */
    void ResetToDefaults();

    // Binding queries
    const InputBinding& GetPrimaryBinding(InputAction action) const noexcept;
    const InputBinding& GetAlternateBinding(InputAction action) const noexcept;
    const InputBinding& GetGamepadBinding(InputAction action) const noexcept;

    // Binding mutators
    void SetPrimaryBinding(InputAction action, const InputBinding& binding);
    void SetAlternateBinding(InputAction action, const InputBinding& binding);
    void SetGamepadBinding(InputAction action, const InputBinding& binding);

    /**
     * Finds any conflicting action currently bound to the specified binding.
     * Excludes `ignoreAction`. Returns InputAction::Count if no conflict.
     */
    InputAction FindConflict(const InputBinding& binding, InputAction ignoreAction) const noexcept;

    // Rebinding workflow
    bool StartRebinding(InputAction action, DeviceType device, bool isAlternate = false);
    void CancelRebinding() noexcept;
    bool IsRebinding() const noexcept { return m_rebindingActive; }
    InputAction GetRebindingAction() const noexcept { return m_rebindingAction; }
    DeviceType GetRebindingDevice() const noexcept { return m_rebindingDevice; }
    bool IsRebindingAlternate() const noexcept { return m_rebindingIsAlternate; }

    /**
     * Completes active rebinding with the supplied binding.
     * If resolveConflict is true, unbinds conflicting action; otherwise cancels if conflict exists.
     */
    bool ApplyRebinding(const InputBinding& newBinding, bool resolveConflict = true);

    // Persistence
    bool LoadFromFile(const std::string& filepath = "config/input_bindings.json");
    bool SaveToFile(const std::string& filepath = "config/input_bindings.json") const;

    // Sensitivity & deadzone settings
    float GetMouseSensitivity() const noexcept { return m_mouseSensitivity; }
    void SetMouseSensitivity(float sens) noexcept { m_mouseSensitivity = sens; }

    float GetGamepadDeadzone() const noexcept { return m_gamepadDeadzone; }
    void SetGamepadDeadzone(float deadzone) noexcept { m_gamepadDeadzone = deadzone; }

private:
    std::array<ActionBindingPair, ACTION_COUNT> m_kbMouseBindings{};
    std::array<InputBinding, ACTION_COUNT> m_gamepadBindings{};

    float m_mouseSensitivity = 1.0f;
    float m_gamepadDeadzone = 0.20f;

    bool m_rebindingActive = false;
    InputAction m_rebindingAction = InputAction::Count;
    DeviceType m_rebindingDevice = DeviceType::Keyboard;
    bool m_rebindingIsAlternate = false;
};

} // namespace wildbound::input


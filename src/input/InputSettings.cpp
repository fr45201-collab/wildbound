#include "InputSettings.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace wildbound::input {

InputSettings::InputSettings() {
    ResetToDefaults();
}

void InputSettings::ResetToDefaults() {
    m_mouseSensitivity = 1.0f;
    m_gamepadDeadzone = 0.20f;
    m_rebindingActive = false;
    m_rebindingAction = InputAction::Count;

    // Clear existing
    for (size_t i = 0; i < ACTION_COUNT; ++i) {
        m_kbMouseBindings[i].primary = InputBinding{};
        m_kbMouseBindings[i].alternate = InputBinding{};
        m_gamepadBindings[i] = InputBinding{};
    }

    // ------------------------------------------------------------------------
    // Keyboard & Mouse Default Bindings
    // ------------------------------------------------------------------------
    auto setKb = [this](InputAction act, int primaryKey, int altKey = 0) {
        size_t idx = static_cast<size_t>(act);
        m_kbMouseBindings[idx].primary = InputBinding{ DeviceType::Keyboard, BindingSourceType::ButtonOrKey, primaryKey };
        if (altKey != 0) {
            m_kbMouseBindings[idx].alternate = InputBinding{ DeviceType::Keyboard, BindingSourceType::ButtonOrKey, altKey };
        }
    };

    auto setMouse = [this](InputAction act, int mouseBtn, int altKey = 0) {
        size_t idx = static_cast<size_t>(act);
        m_kbMouseBindings[idx].primary = InputBinding{ DeviceType::Mouse, BindingSourceType::ButtonOrKey, mouseBtn };
        if (altKey != 0) {
            m_kbMouseBindings[idx].alternate = InputBinding{ DeviceType::Keyboard, BindingSourceType::ButtonOrKey, altKey };
        }
    };

    // Movement
    setKb(InputAction::MoveUp, KEY_W, KEY_UP);
    setKb(InputAction::MoveDown, KEY_S, KEY_DOWN);
    setKb(InputAction::MoveLeft, KEY_A, KEY_LEFT);
    setKb(InputAction::MoveRight, KEY_D, KEY_RIGHT);
    setKb(InputAction::Sprint, KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT);
    setKb(InputAction::Walk, KEY_LEFT_CONTROL, 0);

    // Combat & Interaction
    setMouse(InputAction::PrimaryAttack, MOUSE_BUTTON_LEFT, KEY_SPACE);
    setMouse(InputAction::SecondaryAttack, MOUSE_BUTTON_RIGHT, 0);
    setKb(InputAction::Interact, KEY_E, KEY_F);
    setKb(InputAction::Aim, 0, 0); // Implicit mouse position
    setKb(InputAction::WeaponSwitch, KEY_Q, 0);

    // Inventory & Quick Slots
    setKb(InputAction::OpenInventory, KEY_I, KEY_TAB);
    setKb(InputAction::UseItem, KEY_ENTER, 0);
    setKb(InputAction::DropItem, KEY_G, 0);
    setKb(InputAction::NextInventorySlot, KEY_RIGHT_BRACKET, 0);
    setKb(InputAction::PreviousInventorySlot, KEY_LEFT_BRACKET, 0);
    setKb(InputAction::Hotbar1, KEY_ONE, 0);
    setKb(InputAction::Hotbar2, KEY_TWO, 0);
    setKb(InputAction::Hotbar3, KEY_THREE, 0);
    setKb(InputAction::Hotbar4, KEY_FOUR, 0);

    // Character & World
    setKb(InputAction::OpenCharacter, KEY_C, 0);
    setKb(InputAction::OpenWardrobe, KEY_C, 0);
    setKb(InputAction::OpenMap, KEY_M, 0);

    // General
    setKb(InputAction::Pause, KEY_ESCAPE, 0);
    setKb(InputAction::OpenSettings, KEY_O, 0);
    setKb(InputAction::ToggleMap, KEY_M, 0);

    // UI Navigation
    setKb(InputAction::UIConfirm, KEY_ENTER, KEY_SPACE);
    setKb(InputAction::UICancel, KEY_ESCAPE, 0);
    setKb(InputAction::UINavigateUp, KEY_UP, KEY_W);
    setKb(InputAction::UINavigateDown, KEY_DOWN, KEY_S);
    setKb(InputAction::UINavigateLeft, KEY_LEFT, KEY_A);
    setKb(InputAction::UINavigateRight, KEY_RIGHT, KEY_D);
    setKb(InputAction::UITabNext, KEY_TAB, 0);
    setKb(InputAction::UITabPrevious, KEY_TAB, 0); // With Shift modifier in practice

    // Developer / Diagnostics (Phase 0 - 20 compatibility)
    setKb(InputAction::ToggleDevUI, KEY_F1, 0);
    setKb(InputAction::ToggleCameraMode, KEY_F2, 0);
    setKb(InputAction::ToggleCollisionDebug, KEY_F3, 0);
    setKb(InputAction::ToggleOriginDebug, KEY_F4, 0);
    setKb(InputAction::ToggleAnimDebug, KEY_F5, 0);
    setKb(InputAction::ToggleCombatDebug, KEY_F6, 0);
    setKb(InputAction::ToggleEnemySpawner, KEY_F7, 0);
    setKb(InputAction::ResetPlayer, KEY_R, 0);
    setKb(InputAction::CameraPanUp, KEY_W, 0);
    setKb(InputAction::CameraPanDown, KEY_S, 0);
    setKb(InputAction::CameraPanLeft, KEY_A, 0);
    setKb(InputAction::CameraPanRight, KEY_D, 0);
    setKb(InputAction::CameraZoomIn, KEY_E, 0);
    setKb(InputAction::CameraZoomOut, KEY_Q, 0);

    // ------------------------------------------------------------------------
    // Gamepad Default Bindings
    // ------------------------------------------------------------------------
    auto setGpBtn = [this](InputAction act, int btn) {
        size_t idx = static_cast<size_t>(act);
        m_gamepadBindings[idx] = InputBinding{ DeviceType::Gamepad, BindingSourceType::ButtonOrKey, btn };
    };

    auto setGpAxis = [this](InputAction act, int axis, int dir) {
        size_t idx = static_cast<size_t>(act);
        InputBinding b{ DeviceType::Gamepad, BindingSourceType::Axis };
        b.axis = axis;
        b.axisDirection = dir;
        m_gamepadBindings[idx] = b;
    };

    // Movement (Left Stick & D-Pad)
    setGpAxis(InputAction::MoveUp, GAMEPAD_AXIS_LEFT_Y, -1);
    setGpAxis(InputAction::MoveDown, GAMEPAD_AXIS_LEFT_Y, 1);
    setGpAxis(InputAction::MoveLeft, GAMEPAD_AXIS_LEFT_X, -1);
    setGpAxis(InputAction::MoveRight, GAMEPAD_AXIS_LEFT_X, 1);
    setGpBtn(InputAction::Sprint, GAMEPAD_BUTTON_LEFT_THUMB); // L3

    // Combat
    setGpAxis(InputAction::PrimaryAttack, GAMEPAD_AXIS_RIGHT_TRIGGER, 1); // RT
    setGpAxis(InputAction::SecondaryAttack, GAMEPAD_AXIS_LEFT_TRIGGER, 1); // LT
    setGpBtn(InputAction::Interact, GAMEPAD_BUTTON_RIGHT_FACE_DOWN); // A / Cross
    setGpBtn(InputAction::WeaponSwitch, GAMEPAD_BUTTON_RIGHT_TRIGGER_1); // RB

    // Menus & UI
    setGpBtn(InputAction::OpenInventory, GAMEPAD_BUTTON_RIGHT_FACE_UP); // Y / Triangle
    setGpBtn(InputAction::OpenCharacter, GAMEPAD_BUTTON_RIGHT_FACE_LEFT); // X / Square
    setGpBtn(InputAction::Pause, GAMEPAD_BUTTON_MIDDLE_RIGHT); // Start
    setGpBtn(InputAction::UIConfirm, GAMEPAD_BUTTON_RIGHT_FACE_DOWN); // A
    setGpBtn(InputAction::UICancel, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT); // B / Circle
    setGpBtn(InputAction::UINavigateUp, GAMEPAD_BUTTON_LEFT_FACE_UP);
    setGpBtn(InputAction::UINavigateDown, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
    setGpBtn(InputAction::UINavigateLeft, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
    setGpBtn(InputAction::UINavigateRight, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
    setGpBtn(InputAction::UITabNext, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
    setGpBtn(InputAction::UITabPrevious, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
}

const InputBinding& InputSettings::GetPrimaryBinding(InputAction action) const noexcept {
    static InputBinding invalid;
    size_t idx = static_cast<size_t>(action);
    if (idx < ACTION_COUNT) return m_kbMouseBindings[idx].primary;
    return invalid;
}

const InputBinding& InputSettings::GetAlternateBinding(InputAction action) const noexcept {
    static InputBinding invalid;
    size_t idx = static_cast<size_t>(action);
    if (idx < ACTION_COUNT) return m_kbMouseBindings[idx].alternate;
    return invalid;
}

const InputBinding& InputSettings::GetGamepadBinding(InputAction action) const noexcept {
    static InputBinding invalid;
    size_t idx = static_cast<size_t>(action);
    if (idx < ACTION_COUNT) return m_gamepadBindings[idx];
    return invalid;
}

void InputSettings::SetPrimaryBinding(InputAction action, const InputBinding& binding) {
    size_t idx = static_cast<size_t>(action);
    if (idx < ACTION_COUNT) {
        m_kbMouseBindings[idx].primary = binding;
    }
}

void InputSettings::SetAlternateBinding(InputAction action, const InputBinding& binding) {
    size_t idx = static_cast<size_t>(action);
    if (idx < ACTION_COUNT) {
        m_kbMouseBindings[idx].alternate = binding;
    }
}

void InputSettings::SetGamepadBinding(InputAction action, const InputBinding& binding) {
    size_t idx = static_cast<size_t>(action);
    if (idx < ACTION_COUNT) {
        m_gamepadBindings[idx] = binding;
    }
}

InputAction InputSettings::FindConflict(const InputBinding& binding, InputAction ignoreAction) const noexcept {
    if (!binding.IsValid()) return InputAction::Count;

    for (size_t i = 0; i < ACTION_COUNT; ++i) {
        auto act = static_cast<InputAction>(i);
        if (act == ignoreAction) continue;

        if (binding.device == DeviceType::Gamepad) {
            if (m_gamepadBindings[i] == binding) return act;
        } else {
            if (m_kbMouseBindings[i].primary == binding || m_kbMouseBindings[i].alternate == binding) {
                return act;
            }
        }
    }
    return InputAction::Count;
}

bool InputSettings::StartRebinding(InputAction action, DeviceType device, bool isAlternate) {
    if (action >= InputAction::Count) return false;
    m_rebindingActive = true;
    m_rebindingAction = action;
    m_rebindingDevice = device;
    m_rebindingIsAlternate = isAlternate;
    return true;
}

void InputSettings::CancelRebinding() noexcept {
    m_rebindingActive = false;
    m_rebindingAction = InputAction::Count;
}

bool InputSettings::ApplyRebinding(const InputBinding& newBinding, bool resolveConflict) {
    if (!m_rebindingActive || m_rebindingAction >= InputAction::Count) return false;

    if (!newBinding.IsValid()) {
        CancelRebinding();
        return false;
    }

    InputAction conflict = FindConflict(newBinding, m_rebindingAction);
    if (conflict != InputAction::Count) {
        if (resolveConflict) {
            // Unbind conflicting action
            size_t cIdx = static_cast<size_t>(conflict);
            if (newBinding.device == DeviceType::Gamepad) {
                m_gamepadBindings[cIdx] = InputBinding{};
            } else {
                if (m_kbMouseBindings[cIdx].primary == newBinding) {
                    m_kbMouseBindings[cIdx].primary = InputBinding{};
                }
                if (m_kbMouseBindings[cIdx].alternate == newBinding) {
                    m_kbMouseBindings[cIdx].alternate = InputBinding{};
                }
            }
        } else {
            // Reject due to unresolved conflict
            CancelRebinding();
            return false;
        }
    }

    size_t idx = static_cast<size_t>(m_rebindingAction);
    if (m_rebindingDevice == DeviceType::Gamepad) {
        m_gamepadBindings[idx] = newBinding;
    } else {
        if (m_rebindingIsAlternate) {
            m_kbMouseBindings[idx].alternate = newBinding;
        } else {
            m_kbMouseBindings[idx].primary = newBinding;
        }
    }

    CancelRebinding();
    return true;
}

bool InputSettings::SaveToFile(const std::string& filepath) const {
    try {
        std::filesystem::path outPath(filepath);
        if (outPath.has_parent_path()) {
            std::filesystem::create_directories(outPath.parent_path());
        }

        nlohmann::json root;
        root["version"] = 1;
        root["mouse_sensitivity"] = m_mouseSensitivity;
        root["gamepad_deadzone"] = m_gamepadDeadzone;

        nlohmann::json kbBindingsJson = nlohmann::json::object();
        for (size_t i = 0; i < ACTION_COUNT; ++i) {
            auto act = static_cast<InputAction>(i);
            std::string actName(ActionToString(act));

            nlohmann::json pairJson;
            const auto& p = m_kbMouseBindings[i].primary;
            if (p.IsValid()) {
                pairJson["primary"] = {
                    { "device", p.device == DeviceType::Keyboard ? "keyboard" : "mouse" },
                    { "source", p.source == BindingSourceType::ButtonOrKey ? "button" : "axis" },
                    { "code", p.code },
                    { "axis", p.axis },
                    { "axisDir", p.axisDirection },
                    { "modifiers", p.modifiers }
                };
            }
            const auto& a = m_kbMouseBindings[i].alternate;
            if (a.IsValid()) {
                pairJson["alternate"] = {
                    { "device", a.device == DeviceType::Keyboard ? "keyboard" : "mouse" },
                    { "source", a.source == BindingSourceType::ButtonOrKey ? "button" : "axis" },
                    { "code", a.code },
                    { "axis", a.axis },
                    { "axisDir", a.axisDirection },
                    { "modifiers", a.modifiers }
                };
            }
            kbBindingsJson[actName] = pairJson;
        }
        root["keyboard_mouse_bindings"] = kbBindingsJson;

        nlohmann::json gpBindingsJson = nlohmann::json::object();
        for (size_t i = 0; i < ACTION_COUNT; ++i) {
            auto act = static_cast<InputAction>(i);
            std::string actName(ActionToString(act));
            const auto& b = m_gamepadBindings[i];
            if (b.IsValid()) {
                gpBindingsJson[actName] = {
                    { "device", "gamepad" },
                    { "source", b.source == BindingSourceType::ButtonOrKey ? "button" : "axis" },
                    { "code", b.code },
                    { "axis", b.axis },
                    { "axisDir", b.axisDirection }
                };
            }
        }
        root["gamepad_bindings"] = gpBindingsJson;

        std::ofstream file(filepath);
        if (!file.is_open()) return false;
        file << root.dump(4);
        return true;
    } catch (...) {
        return false;
    }
}

bool InputSettings::LoadFromFile(const std::string& filepath) {
    try {
        if (!std::filesystem::exists(filepath)) {
            return false;
        }

        std::ifstream file(filepath);
        if (!file.is_open()) return false;

        nlohmann::json root = nlohmann::json::parse(file);
        if (root.contains("mouse_sensitivity")) {
            m_mouseSensitivity = root["mouse_sensitivity"].get<float>();
        }
        if (root.contains("gamepad_deadzone")) {
            m_gamepadDeadzone = root["gamepad_deadzone"].get<float>();
        }

        auto parseBinding = [](const nlohmann::json& j) -> InputBinding {
            InputBinding b;
            std::string devStr = j.value("device", "keyboard");
            b.device = (devStr == "mouse") ? DeviceType::Mouse :
                       (devStr == "gamepad") ? DeviceType::Gamepad : DeviceType::Keyboard;
            std::string srcStr = j.value("source", "button");
            b.source = (srcStr == "axis") ? BindingSourceType::Axis : BindingSourceType::ButtonOrKey;
            b.code = j.value("code", 0);
            b.axis = j.value("axis", 0);
            b.axisDirection = j.value("axisDir", 0);
            b.modifiers = static_cast<uint8_t>(j.value("modifiers", 0));
            return b;
        };

        if (root.contains("keyboard_mouse_bindings") && root["keyboard_mouse_bindings"].is_object()) {
            for (auto& [actName, pairJson] : root["keyboard_mouse_bindings"].items()) {
                InputAction act = StringToAction(actName);
                if (act >= InputAction::Count) continue;
                size_t idx = static_cast<size_t>(act);

                if (pairJson.contains("primary")) {
                    m_kbMouseBindings[idx].primary = parseBinding(pairJson["primary"]);
                } else {
                    m_kbMouseBindings[idx].primary = InputBinding{};
                }
                if (pairJson.contains("alternate")) {
                    m_kbMouseBindings[idx].alternate = parseBinding(pairJson["alternate"]);
                } else {
                    m_kbMouseBindings[idx].alternate = InputBinding{};
                }
            }
        }

        if (root.contains("gamepad_bindings") && root["gamepad_bindings"].is_object()) {
            for (auto& [actName, bJson] : root["gamepad_bindings"].items()) {
                InputAction act = StringToAction(actName);
                if (act >= InputAction::Count) continue;
                size_t idx = static_cast<size_t>(act);
                m_gamepadBindings[idx] = parseBinding(bJson);
            }
        }

        return true;
    } catch (...) {
        return false;
    }
}

} // namespace wildbound::input

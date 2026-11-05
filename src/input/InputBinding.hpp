#pragma once

#include "InputAction.hpp"
#include <raylib.h>
#include <vector>
#include <string>
#include <string_view>
#include <array>

namespace wildbound::input {

enum class DeviceType : uint8_t {
    Keyboard,
    Mouse,
    Gamepad
};

enum class BindingSourceType : uint8_t {
    ButtonOrKey,
    Axis
};

enum ModifierFlags : uint8_t {
    Mod_None  = 0,
    Mod_Shift = 1 << 0,
    Mod_Ctrl  = 1 << 1,
    Mod_Alt   = 1 << 2
};

struct InputBinding {
    DeviceType device = DeviceType::Keyboard;
    BindingSourceType source = BindingSourceType::ButtonOrKey;
    int code = 0;              // KeyboardKey, MouseButton, or GamepadButton
    int axis = 0;              // GamepadAxis or MouseWheel
    int axisDirection = 0;     // +1 or -1
    float axisThreshold = 0.5f;
    uint8_t modifiers = Mod_None;

    bool operator==(const InputBinding& other) const noexcept {
        return device == other.device &&
               source == other.source &&
               code == other.code &&
               axis == other.axis &&
               axisDirection == other.axisDirection &&
               modifiers == other.modifiers;
    }

    bool IsValid() const noexcept {
        if (source == BindingSourceType::ButtonOrKey) {
            if (device == DeviceType::Mouse) return code >= 0;
            return code != 0;
        } else {
            return axisDirection != 0;
        }
    }
};

inline std::string KeyCodeToString(int keyCode) {
    switch (keyCode) {
        case KEY_W: return "W";
        case KEY_A: return "A";
        case KEY_S: return "S";
        case KEY_D: return "D";
        case KEY_E: return "E";
        case KEY_F: return "F";
        case KEY_I: return "I";
        case KEY_C: return "C";
        case KEY_M: return "M";
        case KEY_Q: return "Q";
        case KEY_R: return "R";
        case KEY_UP: return "Up";
        case KEY_DOWN: return "Down";
        case KEY_LEFT: return "Left";
        case KEY_RIGHT: return "Right";
        case KEY_SPACE: return "Space";
        case KEY_ESCAPE: return "Escape";
        case KEY_ENTER: return "Enter";
        case KEY_TAB: return "Tab";
        case KEY_LEFT_SHIFT: return "LShift";
        case KEY_RIGHT_SHIFT: return "RShift";
        case KEY_LEFT_CONTROL: return "LCtrl";
        case KEY_RIGHT_CONTROL: return "RCtrl";
        case KEY_ONE: return "1";
        case KEY_TWO: return "2";
        case KEY_THREE: return "3";
        case KEY_FOUR: return "4";
        case KEY_F1: return "F1";
        case KEY_F2: return "F2";
        case KEY_F3: return "F3";
        case KEY_F4: return "F4";
        case KEY_F5: return "F5";
        case KEY_F6: return "F6";
        case KEY_F7: return "F7";
        default: return "Key_" + std::to_string(keyCode);
    }
}

inline std::string MouseButtonToString(int button) {
    switch (button) {
        case MOUSE_BUTTON_LEFT: return "MouseLeft";
        case MOUSE_BUTTON_RIGHT: return "MouseRight";
        case MOUSE_BUTTON_MIDDLE: return "MouseMiddle";
        case MOUSE_BUTTON_SIDE: return "MouseSide";
        case MOUSE_BUTTON_EXTRA: return "MouseExtra";
        default: return "Mouse_" + std::to_string(button);
    }
}

inline std::string GamepadButtonToString(int button) {
    switch (button) {
        case GAMEPAD_BUTTON_LEFT_FACE_UP: return "D-Pad Up";
        case GAMEPAD_BUTTON_LEFT_FACE_RIGHT: return "D-Pad Right";
        case GAMEPAD_BUTTON_LEFT_FACE_DOWN: return "D-Pad Down";
        case GAMEPAD_BUTTON_LEFT_FACE_LEFT: return "D-Pad Left";
        case GAMEPAD_BUTTON_RIGHT_FACE_UP: return "Y / Triangle";
        case GAMEPAD_BUTTON_RIGHT_FACE_RIGHT: return "B / Circle";
        case GAMEPAD_BUTTON_RIGHT_FACE_DOWN: return "A / Cross";
        case GAMEPAD_BUTTON_RIGHT_FACE_LEFT: return "X / Square";
        case GAMEPAD_BUTTON_LEFT_TRIGGER_1: return "LB / L1";
        case GAMEPAD_BUTTON_LEFT_TRIGGER_2: return "LT / L2";
        case GAMEPAD_BUTTON_RIGHT_TRIGGER_1: return "RB / R1";
        case GAMEPAD_BUTTON_RIGHT_TRIGGER_2: return "RT / R2";
        case GAMEPAD_BUTTON_MIDDLE_LEFT: return "Back / Select";
        case GAMEPAD_BUTTON_MIDDLE: return "Home / Guide";
        case GAMEPAD_BUTTON_MIDDLE_RIGHT: return "Start / Options";
        case GAMEPAD_BUTTON_LEFT_THUMB: return "LS / L3";
        case GAMEPAD_BUTTON_RIGHT_THUMB: return "RS / R3";
        default: return "Gamepad_" + std::to_string(button);
    }
}

inline std::string BindingToString(const InputBinding& b) {
    if (!b.IsValid()) return "None";

    std::string prefix = "";
    if (b.modifiers & Mod_Ctrl)  prefix += "Ctrl+";
    if (b.modifiers & Mod_Shift) prefix += "Shift+";
    if (b.modifiers & Mod_Alt)   prefix += "Alt+";

    if (b.device == DeviceType::Keyboard) {
        return prefix + KeyCodeToString(b.code);
    } else if (b.device == DeviceType::Mouse) {
        if (b.source == BindingSourceType::ButtonOrKey) {
            return prefix + MouseButtonToString(b.code);
        } else {
            return prefix + (b.axisDirection > 0 ? "WheelUp" : "WheelDown");
        }
    } else if (b.device == DeviceType::Gamepad) {
        if (b.source == BindingSourceType::ButtonOrKey) {
            return prefix + GamepadButtonToString(b.code);
        } else {
            std::string axisName = (b.axis == GAMEPAD_AXIS_LEFT_X) ? "LS_X" :
                                   (b.axis == GAMEPAD_AXIS_LEFT_Y) ? "LS_Y" :
                                   (b.axis == GAMEPAD_AXIS_RIGHT_X) ? "RS_X" :
                                   (b.axis == GAMEPAD_AXIS_RIGHT_Y) ? "RS_Y" :
                                   (b.axis == GAMEPAD_AXIS_LEFT_TRIGGER) ? "LT" :
                                   (b.axis == GAMEPAD_AXIS_RIGHT_TRIGGER) ? "RT" : "Axis";
            return prefix + axisName + (b.axisDirection > 0 ? "+" : "-");
        }
    }
    return "Unknown";
}

} // namespace wildbound::input

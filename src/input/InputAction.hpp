#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace wildbound::input {

/**
 * Logical action categories for context filtering and organization.
 */
enum class ActionCategory : uint8_t {
    Movement,
    Combat,
    Inventory,
    Character,
    UI,
    General,
    Debug,
    Count
};

/**
 * Authoritative enumerations for all logical input actions in Wildbound.
 * Gameplay and UI systems consume these actions rather than polling physical keys.
 */
enum class InputAction : uint8_t {
    // Locomotion
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    Sprint,
    Walk,

    // Combat & Interaction
    PrimaryAttack,
    SecondaryAttack,
    WeaponSwitch,
    Aim,
    Interact,

    // Inventory & Quick Slots
    OpenInventory,
    UseItem,
    DropItem,
    NextInventorySlot,
    PreviousInventorySlot,
    Hotbar1,
    Hotbar2,
    Hotbar3,
    Hotbar4,

    // Character & World
    OpenCharacter,
    OpenWardrobe,
    OpenMap,

    // General & System
    Pause,
    OpenSettings,
    ToggleMap,

    // UI Navigation
    UIConfirm,
    UICancel,
    UINavigateUp,
    UINavigateDown,
    UINavigateLeft,
    UINavigateRight,
    UITabNext,
    UITabPrevious,

    // Developer / Diagnostic (Maintains backward compatibility with F1-F7)
    ToggleDevUI,
    ToggleCameraMode,
    ToggleCollisionDebug,
    ToggleOriginDebug,
    ToggleAnimDebug,
    ToggleCombatDebug,
    ToggleEnemySpawner,
    ResetPlayer,
    CameraPanUp,
    CameraPanDown,
    CameraPanLeft,
    CameraPanRight,
    CameraZoomIn,
    CameraZoomOut,

    Count
};

constexpr size_t ACTION_COUNT = static_cast<size_t>(InputAction::Count);

inline ActionCategory GetActionCategory(InputAction action) noexcept {
    switch (action) {
        case InputAction::MoveUp:
        case InputAction::MoveDown:
        case InputAction::MoveLeft:
        case InputAction::MoveRight:
        case InputAction::Sprint:
        case InputAction::Walk:
            return ActionCategory::Movement;

        case InputAction::PrimaryAttack:
        case InputAction::SecondaryAttack:
        case InputAction::WeaponSwitch:
        case InputAction::Aim:
        case InputAction::Interact:
            return ActionCategory::Combat;

        case InputAction::OpenInventory:
        case InputAction::UseItem:
        case InputAction::DropItem:
        case InputAction::NextInventorySlot:
        case InputAction::PreviousInventorySlot:
        case InputAction::Hotbar1:
        case InputAction::Hotbar2:
        case InputAction::Hotbar3:
        case InputAction::Hotbar4:
            return ActionCategory::Inventory;

        case InputAction::OpenCharacter:
        case InputAction::OpenWardrobe:
        case InputAction::OpenMap:
            return ActionCategory::Character;

        case InputAction::UIConfirm:
        case InputAction::UICancel:
        case InputAction::UINavigateUp:
        case InputAction::UINavigateDown:
        case InputAction::UINavigateLeft:
        case InputAction::UINavigateRight:
        case InputAction::UITabNext:
        case InputAction::UITabPrevious:
            return ActionCategory::UI;

        case InputAction::Pause:
        case InputAction::OpenSettings:
        case InputAction::ToggleMap:
            return ActionCategory::General;

        case InputAction::ToggleDevUI:
        case InputAction::ToggleCameraMode:
        case InputAction::ToggleCollisionDebug:
        case InputAction::ToggleOriginDebug:
        case InputAction::ToggleAnimDebug:
        case InputAction::ToggleCombatDebug:
        case InputAction::ToggleEnemySpawner:
        case InputAction::ResetPlayer:
        case InputAction::CameraPanUp:
        case InputAction::CameraPanDown:
        case InputAction::CameraPanLeft:
        case InputAction::CameraPanRight:
        case InputAction::CameraZoomIn:
        case InputAction::CameraZoomOut:
            return ActionCategory::Debug;

        default:
            return ActionCategory::General;
    }
}

inline std::string_view ActionToString(InputAction action) noexcept {
    switch (action) {
        case InputAction::MoveUp: return "MoveUp";
        case InputAction::MoveDown: return "MoveDown";
        case InputAction::MoveLeft: return "MoveLeft";
        case InputAction::MoveRight: return "MoveRight";
        case InputAction::Sprint: return "Sprint";
        case InputAction::Walk: return "Walk";
        case InputAction::PrimaryAttack: return "PrimaryAttack";
        case InputAction::SecondaryAttack: return "SecondaryAttack";
        case InputAction::WeaponSwitch: return "WeaponSwitch";
        case InputAction::Aim: return "Aim";
        case InputAction::Interact: return "Interact";
        case InputAction::OpenInventory: return "OpenInventory";
        case InputAction::UseItem: return "UseItem";
        case InputAction::DropItem: return "DropItem";
        case InputAction::NextInventorySlot: return "NextInventorySlot";
        case InputAction::PreviousInventorySlot: return "PreviousInventorySlot";
        case InputAction::Hotbar1: return "Hotbar1";
        case InputAction::Hotbar2: return "Hotbar2";
        case InputAction::Hotbar3: return "Hotbar3";
        case InputAction::Hotbar4: return "Hotbar4";
        case InputAction::OpenCharacter: return "OpenCharacter";
        case InputAction::OpenWardrobe: return "OpenWardrobe";
        case InputAction::OpenMap: return "OpenMap";
        case InputAction::Pause: return "Pause";
        case InputAction::OpenSettings: return "OpenSettings";
        case InputAction::ToggleMap: return "ToggleMap";
        case InputAction::UIConfirm: return "UIConfirm";
        case InputAction::UICancel: return "UICancel";
        case InputAction::UINavigateUp: return "UINavigateUp";
        case InputAction::UINavigateDown: return "UINavigateDown";
        case InputAction::UINavigateLeft: return "UINavigateLeft";
        case InputAction::UINavigateRight: return "UINavigateRight";
        case InputAction::UITabNext: return "UITabNext";
        case InputAction::UITabPrevious: return "UITabPrevious";
        case InputAction::ToggleDevUI: return "ToggleDevUI";
        case InputAction::ToggleCameraMode: return "ToggleCameraMode";
        case InputAction::ToggleCollisionDebug: return "ToggleCollisionDebug";
        case InputAction::ToggleOriginDebug: return "ToggleOriginDebug";
        case InputAction::ToggleAnimDebug: return "ToggleAnimDebug";
        case InputAction::ToggleCombatDebug: return "ToggleCombatDebug";
        case InputAction::ToggleEnemySpawner: return "ToggleEnemySpawner";
        case InputAction::ResetPlayer: return "ResetPlayer";
        case InputAction::CameraPanUp: return "CameraPanUp";
        case InputAction::CameraPanDown: return "CameraPanDown";
        case InputAction::CameraPanLeft: return "CameraPanLeft";
        case InputAction::CameraPanRight: return "CameraPanRight";
        case InputAction::CameraZoomIn: return "CameraZoomIn";
        case InputAction::CameraZoomOut: return "CameraZoomOut";
        default: return "Unknown";
    }
}

inline InputAction StringToAction(std::string_view name) noexcept {
    for (size_t i = 0; i < ACTION_COUNT; ++i) {
        auto act = static_cast<InputAction>(i);
        if (ActionToString(act) == name) {
            return act;
        }
    }
    return InputAction::Count;
}

} // namespace wildbound::input


#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace wildbound::interaction {

using InteractableId = uint64_t;
constexpr InteractableId INVALID_INTERACTABLE_ID = 0;

/**
 * Functional categories of world interactable objects.
 */
enum class InteractionCategory : uint8_t {
    Door,           // Openable/closable hinged doors and gates
    Building,       // Structure entrances and interior transitions
    Container,      // Chests, crates, storage barrels, caches
    Collectible,    // World items, resources, dropped weapons
    Mechanism,      // Levers, switches, pressure plates, wheels
    Environmental,  // Campfires, wells, shrines, monuments, signs
    Special         // Unique boss portals, ancient artifacts
};

inline std::string CategoryToString(InteractionCategory category) {
    switch (category) {
        case InteractionCategory::Door:          return "Door";
        case InteractionCategory::Building:      return "Building";
        case InteractionCategory::Container:     return "Container";
        case InteractionCategory::Collectible:   return "Collectible";
        case InteractionCategory::Mechanism:     return "Mechanism";
        case InteractionCategory::Environmental: return "Environmental";
        case InteractionCategory::Special:       return "Special";
        default:                                 return "Unknown";
    }
}

/**
 * Result outcomes of an interaction attempt.
 */
enum class InteractionResult : uint8_t {
    Success,             // Interaction completed successfully
    OutOfRange,          // Player is outside the interaction radius
    TargetUnavailable,   // Target is disabled, consumed, or empty
    InvalidTarget,       // Target does not exist or has been destroyed
    MissingRequirement,  // Missing required item, key, or condition
    AlreadyExecuting,    // Interaction is currently in-flight
    CooldownActive,      // Interaction is on cooldown
    Blocked,             // Blocked by geometry, combat state, or modal context
    Failed               // General execution failure
};

inline std::string InteractionResultToString(InteractionResult result) {
    switch (result) {
        case InteractionResult::Success:            return "Success";
        case InteractionResult::OutOfRange:         return "Out of Range";
        case InteractionResult::TargetUnavailable:  return "Target Unavailable";
        case InteractionResult::InvalidTarget:      return "Invalid Target";
        case InteractionResult::MissingRequirement: return "Missing Requirement";
        case InteractionResult::AlreadyExecuting:   return "Already Executing";
        case InteractionResult::CooldownActive:     return "Cooldown Active";
        case InteractionResult::Blocked:            return "Blocked";
        case InteractionResult::Failed:             return "Failed";
        default:                                    return "Unknown";
    }
}

/**
 * Policy for collecting world items.
 */
enum class PickupPolicy : uint8_t {
    Manual,    // Requires pressing the Interact action
    Automatic  // Collected immediately upon collision/proximity
};

/**
 * Operational states of doors and gates.
 */
enum class DoorState : uint8_t {
    Closed,
    Open,
    Locked
};

inline std::string DoorStateToString(DoorState state) {
    switch (state) {
        case DoorState::Closed: return "Closed";
        case DoorState::Open:   return "Open";
        case DoorState::Locked: return "Locked";
        default:                return "Unknown";
    }
}

/**
 * Functional mechanism types.
 */
enum class MechanismType : uint8_t {
    Lever,          // Two-state toggle switch
    Switch,         // Momentary push button
    PressurePlate,  // Floor trigger (auto-activated by weight)
    RotatingWheel,  // Valve or turnstile
    GateControl     // Portcullis winch
};

/**
 * Operational states of mechanisms.
 */
enum class MechanismState : uint8_t {
    Inactive,
    Active
};

} // namespace wildbound::interaction


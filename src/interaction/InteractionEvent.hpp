#pragma once

#include "InteractionTypes.hpp"
#include <string>
#include <functional>

namespace wildbound::interaction {

enum class InteractionEventType : uint8_t {
    TargetChanged,        // New interaction target selected or cleared
    InteractionStarted,   // Interaction attempt commenced
    InteractionSucceeded, // Interaction completed successfully
    InteractionFailed,    // Interaction attempt failed validation or execution
    DoorOpened,           // A world door transitioned to open
    DoorClosed,           // A world door transitioned to closed
    BuildingEntered,      // Player entered an interior
    BuildingExited,       // Player returned to the exterior
    ContainerOpened,      // A loot container was opened
    ItemCollected,        // A world item was transferred to inventory
    MechanismActivated,   // A mechanism was toggled on
    MechanismDeactivated  // A mechanism was toggled off
};

struct InteractionEvent {
    InteractionEventType type = InteractionEventType::InteractionSucceeded;
    InteractableId interactableId = INVALID_INTERACTABLE_ID;
    std::string interactionId = "";
    InteractionCategory category = InteractionCategory::Environmental;
    InteractionResult result = InteractionResult::Success;
    std::string detail = "";
    float timestamp = 0.0f;
};

class IInteractionListener {
public:
    virtual ~IInteractionListener() = default;
    virtual void OnInteractionEvent(const InteractionEvent& event) = 0;
};

using InteractionEventCallback = std::function<void(const InteractionEvent&)>;

} // namespace wildbound::interaction


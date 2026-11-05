#pragma once

#include "InteractionTypes.hpp"
#include <string>

namespace wildbound::interaction {

/**
 * Data-driven configuration defining how an object presents and behaves during interaction.
 */
struct InteractionDefinition {
    std::string id;                                     // Unique identifier (e.g., "forest_chest_001")
    std::string displayName = "Interactable Object";    // In-game name (e.g., "Ironbound Chest")
    std::string description = "";                       // Contextual lore or detail
    std::string verb = "Interact";                      // Prompt verb (e.g., "Open", "Enter", "Collect")
    InteractionCategory category = InteractionCategory::Environmental;
    
    float range = 48.0f;                                // Proximity radius in virtual pixels
    int priority = 10;                                  // Tie-breaking priority (higher evaluated first)
    float cooldown = 0.25f;                             // Cooldown between repeated triggers (seconds)
    bool enabled = true;                                // Whether candidate can be discovered/executed
    
    std::string requiredItemId = "";                    // Required key or quest item to unlock
    std::string audioEvent = "";                        // Dispatched audio event (e.g., "Interact.ChestOpen")
    bool oneTimeOnly = false;                           // Disables itself after successful execution
};

} // namespace wildbound::interaction


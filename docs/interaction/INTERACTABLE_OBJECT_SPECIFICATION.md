# Interactable Object Specification

## 1. Overview

An interactable object in Wildbound is any entity or world element with which the player can trigger a contextual action. Objects are defined data-first using `InteractionDefinition` and tracked at runtime using `InteractableComponent`.

---

## 2. Categories (`InteractionCategory`)

| Category | Description | Primary Verbs | Linked Subsystem / Entity |
| :--- | :--- | :--- | :--- |
| `Door` | Physical passageway barrier | `Open`, `Close`, `Unlock` | `Door` entity, `CollisionManager` |
| `Building` | Interior/exterior building entrance | `Enter`, `Exit` | `BuildingManager`, `WorldRenderer` |
| `Container` | Lootable chest or crate | `Open`, `Loot` | `LootContainer`, `Inventory` |
| `Collectible` | Physical item lying on ground | `Collect`, `Take` | `WorldItem`, `Inventory` |
| `Mechanism` | Actuators (lever, switch, pressure plate) | `Activate`, `Pull`, `Trigger` | `Mechanism`, game triggers |
| `Environmental`| Atmospheric world objects (campfires, wells) | `Inspect`, `Rest`, `Pray` | World environment objects |
| `Special` | Shrines, portals, ritual stones | `Channel`, `Commune` | Custom gameplay scripts |

---

## 3. Data Structures

### 3.1 `InteractionDefinition`
```cpp
struct InteractionDefinition {
    std::string id;                                     // Unique definition key (e.g., "door_oak_01")
    std::string displayName = "Interactable Object";    // Player-facing name (e.g., "Sturdy Oak Door")
    std::string description = "";                       // Contextual lore or detail
    std::string verb = "Interact";                      // Default action verb (e.g., "Open")
    InteractionCategory category = InteractionCategory::Environmental;
    
    float range = 48.0f;                                // Proximity radius in virtual pixels
    int priority = 10;                                  // Selection priority (higher evaluates first)
    float cooldown = 0.25f;                             // Cooldown between repeated triggers (seconds)
    bool enabled = true;                                // Whether candidate can be discovered/executed
    
    std::string requiredItemId = "";                    // Required key or quest item to unlock
    std::string audioEvent = "";                        // Dispatched audio event (e.g., "Interact.ChestOpen")
    bool oneTimeOnly = false;                           // Disables itself after successful execution
};
```

### 3.2 `InteractableComponent`
```cpp
struct InteractableComponent {
    InteractableId id = INVALID_INTERACTABLE_ID;        // Unique runtime handle (uint64_t)
    std::string linkedEntityId = "";                    // Associated domain ID (e.g., chest or door ID)
    Vector2 position{ 0.0f, 0.0f };                     // World anchor position
    
    InteractionDefinition definition;                   // Data-driven configuration
    
    bool isAvailable = true;                            // Runtime enablement flag
    bool isExecuting = false;                           // Active execution lock flag
    bool hasBeenExecuted = false;                       // One-time execution tracking
    float currentCooldown = 0.0f;                       // Remaining cooldown timer (seconds)
    
    InteractionCallback customHandler = nullptr;        // Optional lambda callback override
};
```

---

## 4. Lifecycle & Registration

1. **Static Map Objects**: Registered during map loading or world initialization (`InteractionManager::RegisterDoor`, `RegisterMechanism`).
2. **Dynamic World Objects**: Automatically synchronized each frame via `InteractionManager::SyncWithWorld()`:
   - Dynamic `LootContainer` instances in `World` mapped to `InteractionCategory::Container`.
   - Active `WorldItem` instances in `World` mapped to `InteractionCategory::Collectible`.
   - Active building entrance/exit zones mapped to `InteractionCategory::Building`.
3. **Deregistration**: World items mark themselves collected and are culled from active interactables automatically. Mechanisms or chests marked `oneTimeOnly` deactivate their `isAvailable` flag upon successful execution.


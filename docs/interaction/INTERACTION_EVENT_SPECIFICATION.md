# Interaction Event Specification

## 1. Overview

The interaction subsystem implements an observer-based event notification bus (`IInteractionObserver`). External subsystems register as observers to listen for interaction lifecycle events without circular header dependencies or tight coupling.

---

## 2. Event Types (`InteractionEventType`)

| Event Type | Trigger Condition | Common Payload Details |
| :--- | :--- | :--- |
| `TargetChanged` | Player moves and best candidate changes | New target ID, display name, category |
| `InteractionStarted` | Player presses interact; validation succeeded | Target ID, action verb |
| `InteractionSucceeded`| Interaction completed successfully | Result code, transferred quantity |
| `InteractionFailed` | Validation failed or execution blocked | Failure code, descriptive message |
| `DoorOpened` | Door transitioned to Open | Door ID, solid collider status |
| `DoorClosed` | Door transitioned to Closed | Door ID, solid collider status |
| `BuildingEntered` | Player entered building interior | Building ID, interior scene |
| `BuildingExited` | Player exited back to exterior map | Building ID, exterior coordinates |
| `ContainerOpened` | Loot chest opened | Container ID, items transferred |
| `ItemCollected` | Ground item picked up into inventory | Item ID, definition, count |
| `MechanismActivated` | Lever or switch activated | Mechanism ID, active state |
| `MechanismDeactivated`| Lever or switch deactivated | Mechanism ID, active state |

---

## 3. Event Payload Structure (`InteractionEvent`)

```cpp
struct InteractionEvent {
    InteractionEventType type = InteractionEventType::InteractionSucceeded;
    InteractableId interactableId = INVALID_INTERACTABLE_ID;
    std::string interactionId = "";
    InteractionCategory category = InteractionCategory::Environmental;
    InteractionResult result = InteractionResult::Success;
    std::string detail = "";
    float timestamp = 0.0f;
};
```

---

## 4. Observer Interface (`IInteractionObserver`)

```cpp
class IInteractionObserver {
public:
    virtual ~IInteractionObserver() = default;
    virtual void OnInteractionEvent(const InteractionEvent& event) = 0;
};
```
Subsystems like `AudioSystem`, quest managers, achievement trackers, and game telemetry register via:
```cpp
interactionManager.AddObserver(&audioObserver);
```


#include "InteractionManager.hpp"
#include "InteractionValidator.hpp"
#include "InteractionExecutor.hpp"
#include "../input/InputManager.hpp"
#include "../entities/Player.hpp"
#include "../world/World.hpp"
#include "../world/BuildingManager.hpp"
#include "../world/LootContainer.hpp"
#include "../world/WorldItem.hpp"
#include "../items/ItemManager.hpp"
#include "../core/Logger.hpp"
#include <algorithm>
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::interaction {

InteractionManager::InteractionManager() {
    m_currentCandidates.reserve(16);
}

InteractableId InteractionManager::RegisterInteractable(InteractableComponent component) {
    if (component.id == INVALID_INTERACTABLE_ID) {
        component.id = m_nextInteractableId++;
    } else if (component.id >= m_nextInteractableId) {
        m_nextInteractableId = component.id + 1;
    }

    InteractableId assignedId = component.id;
    m_interactables[assignedId] = std::move(component);
    return assignedId;
}

void InteractionManager::UnregisterInteractable(InteractableId id) {
    m_interactables.erase(id);
    if (m_currentTarget && m_currentTarget->id == id) {
        m_currentTarget = nullptr;
        m_currentPrompt.visible = false;
    }
}

InteractableComponent* InteractionManager::GetInteractable(InteractableId id) {
    auto it = m_interactables.find(id);
    return (it != m_interactables.end()) ? &it->second : nullptr;
}

const InteractableComponent* InteractionManager::GetInteractable(InteractableId id) const {
    auto it = m_interactables.find(id);
    return (it != m_interactables.end()) ? &it->second : nullptr;
}

void InteractionManager::ClearInteractables() {
    m_interactables.clear();
    m_currentTarget = nullptr;
    m_currentCandidates.clear();
    m_currentPrompt.visible = false;
    m_lastTargetId = INVALID_INTERACTABLE_ID;
}

Door* InteractionManager::RegisterDoor(Door door) {
    m_doors.push_back(std::move(door));
    Door* ptr = &m_doors.back();

    // Register corresponding interactable component
    InteractableComponent comp;
    comp.position = ptr->GetPosition();
    comp.bounds = ptr->GetBounds();
    comp.linkedEntityId = ptr->GetId();
    comp.definition.id = "door_" + ptr->GetId();
    comp.definition.displayName = ptr->GetName();
    comp.definition.verb = ptr->IsOpen() ? "Close" : "Open";
    comp.definition.category = InteractionCategory::Door;
    comp.definition.range = 52.0f;
    comp.definition.priority = 20; // Doors have higher priority than ground items
    comp.definition.requiredItemId = ptr->GetRequiredKeyId();
    RegisterInteractable(std::move(comp));

    return ptr;
}

Door* InteractionManager::GetDoor(const std::string& id) {
    for (auto& d : m_doors) {
        if (d.GetId() == id) return &d;
    }
    return nullptr;
}

const Door* InteractionManager::GetDoor(const std::string& id) const {
    for (const auto& d : m_doors) {
        if (d.GetId() == id) return &d;
    }
    return nullptr;
}

Mechanism* InteractionManager::RegisterMechanism(Mechanism mechanism) {
    m_mechanisms.push_back(std::move(mechanism));
    Mechanism* ptr = &m_mechanisms.back();

    InteractableComponent comp;
    comp.position = ptr->GetPosition();
    comp.bounds = ptr->GetBounds();
    comp.linkedEntityId = ptr->GetId();
    comp.definition.id = "mechanism_" + ptr->GetId();
    comp.definition.displayName = ptr->GetName();
    comp.definition.verb = ptr->IsActive() ? "Deactivate" : "Activate";
    comp.definition.category = InteractionCategory::Mechanism;
    comp.definition.range = 48.0f;
    comp.definition.priority = 15;
    RegisterInteractable(std::move(comp));

    return ptr;
}

Mechanism* InteractionManager::GetMechanism(const std::string& id) {
    for (auto& m : m_mechanisms) {
        if (m.GetId() == id) return &m;
    }
    return nullptr;
}

const Mechanism* InteractionManager::GetMechanism(const std::string& id) const {
    for (const auto& m : m_mechanisms) {
        if (m.GetId() == id) return &m;
    }
    return nullptr;
}

void InteractionManager::AddListener(IInteractionListener* listener) {
    if (listener && std::find(m_listeners.begin(), m_listeners.end(), listener) == m_listeners.end()) {
        m_listeners.push_back(listener);
    }
}

void InteractionManager::RemoveListener(IInteractionListener* listener) {
    m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
}

void InteractionManager::DispatchEvent(const InteractionEvent& event) {
    for (auto* listener : m_listeners) {
        if (listener) {
            listener->OnInteractionEvent(event);
        }
    }
}

void InteractionManager::Update(float dt, InteractionContext& context, const input::InputManager* inputMgr) {
    // 1. Tick cooldowns and prompt feedback
    for (auto& [id, comp] : m_interactables) {
        comp.Update(dt);
    }
    for (auto& m : m_mechanisms) {
        m.Update(dt);
    }
    m_currentPrompt.Update(dt);

    if (!context.player) {
        m_currentTarget = nullptr;
        m_currentCandidates.clear();
        m_currentPrompt.visible = false;
        return;
    }

    Vector2 playerPos = context.player->GetPosition();
    Vector2 playerFacing = context.player->GetFacingDirection();

    // 2. Perform candidate discovery & ranking
    UpdateDiscovery(playerPos, playerFacing);

    // 3. Update contextual prompt UI
    UpdatePrompt(inputMgr);

    // 4. Evaluate player input
    if (inputMgr) {
        bool inGameplay = (inputMgr->GetContextManager().GetActiveContext().type == input::InputContextType::Gameplay);
        if (inGameplay) {
            bool interactPressed = const_cast<input::InputManager*>(inputMgr)->ConsumeAction(input::InputAction::Interact);
            if (interactPressed && m_currentTarget) {
                TryInteract(context);
            }
        }
    }
}

void InteractionManager::UpdateDiscovery(const Vector2& playerPos, const Vector2& playerFacing) {
    m_currentCandidates.clear();

    for (const auto& [id, comp] : m_interactables) {
        if (!comp.CanInteract()) {
            continue;
        }

        float dx = comp.position.x - playerPos.x;
        float dy = comp.position.y - playerPos.y;
        float distSq = dx * dx + dy * dy;
        float range = comp.definition.range > 0.0f ? comp.definition.range : m_defaultRange;

        if (distSq <= range * range) {
            float dist = std::sqrt(distSq);
            float facingAlignment = 0.0f;
            if (dist > 0.001f) {
                Vector2 dir{ dx / dist, dy / dist };
                facingAlignment = (dir.x * playerFacing.x) + (dir.y * playerFacing.y);
            }

            m_currentCandidates.push_back(CandidateTarget{
                &comp,
                dist,
                facingAlignment,
                comp.definition.priority
            });
        }
    }

    // Deterministic Candidate Sorting Policy:
    // 1. Priority (higher wins)
    // 2. Facing direction (in-front > behind)
    // 3. Proximity (closer distance wins)
    // 4. Stable InteractableId tie-breaker
    std::sort(m_currentCandidates.begin(), m_currentCandidates.end(), [](const CandidateTarget& a, const CandidateTarget& b) {
        if (a.priority != b.priority) {
            return a.priority > b.priority;
        }
        bool aInFront = a.facingAlignment > 0.15f;
        bool bInFront = b.facingAlignment > 0.15f;
        if (aInFront != bInFront) {
            return aInFront;
        }
        if (std::abs(a.distance - b.distance) > 1.0f) {
            return a.distance < b.distance;
        }
        return a.component->id < b.component->id;
    });

    if (!m_currentCandidates.empty()) {
        m_currentTarget = m_currentCandidates.front().component;
    } else {
        m_currentTarget = nullptr;
    }

    // Dispatch TargetChanged event if target switched
    InteractableId currId = m_currentTarget ? m_currentTarget->id : INVALID_INTERACTABLE_ID;
    if (currId != m_lastTargetId) {
        m_lastTargetId = currId;
        InteractionEvent ev;
        ev.type = InteractionEventType::TargetChanged;
        ev.interactableId = currId;
        if (m_currentTarget) {
            ev.interactionId = m_currentTarget->definition.id;
            ev.category = m_currentTarget->definition.category;
            ev.detail = m_currentTarget->definition.displayName;
        }
        DispatchEvent(ev);
    }
}

void InteractionManager::UpdatePrompt(const input::InputManager* inputMgr) {
    if (!m_currentTarget) {
        m_currentPrompt.visible = false;
        return;
    }

    m_currentPrompt.visible = true;
    m_currentPrompt.targetName = m_currentTarget->definition.displayName;
    m_currentPrompt.description = m_currentTarget->definition.description;
    m_currentPrompt.worldPosition = m_currentTarget->position;
    m_currentPrompt.category = m_currentTarget->definition.category;
    m_currentPrompt.distance = m_currentCandidates.empty() ? 0.0f : m_currentCandidates.front().distance;

    // Update dynamic verb based on door/mechanism state
    if (m_currentTarget->definition.category == InteractionCategory::Door) {
        const auto* d = GetDoor(m_currentTarget->linkedEntityId);
        if (d) {
            if (d->IsLocked()) {
                m_currentPrompt.verb = "Unlock";
            } else if (d->IsOpen()) {
                m_currentPrompt.verb = "Close";
            } else {
                m_currentPrompt.verb = "Open";
            }
        } else {
            m_currentPrompt.verb = m_currentTarget->definition.verb;
        }
    } else if (m_currentTarget->definition.category == InteractionCategory::Mechanism) {
        const auto* m = GetMechanism(m_currentTarget->linkedEntityId);
        if (m) {
            m_currentPrompt.verb = m->IsActive() ? "Deactivate" : "Activate";
        } else {
            m_currentPrompt.verb = m_currentTarget->definition.verb;
        }
    } else {
        m_currentPrompt.verb = m_currentTarget->definition.verb;
    }

    // Dynamic Key Glyph from active input device
    if (inputMgr) {
        if (inputMgr->GetActiveDevice() == input::DeviceType::Gamepad) {
            m_currentPrompt.keyGlyph = "X / Square";
        } else {
            m_currentPrompt.keyGlyph = "E";
        }
    } else {
        m_currentPrompt.keyGlyph = "E";
    }
}

InteractionResult InteractionManager::TryInteract(InteractionContext& context) {
    if (!m_currentTarget) {
        return InteractionResult::InvalidTarget;
    }
    return InteractWith(m_currentTarget->id, context);
}

InteractionResult InteractionManager::InteractWith(InteractableId id, InteractionContext& context) {
    auto* target = GetInteractable(id);
    if (!target) {
        return InteractionResult::InvalidTarget;
    }

    Vector2 playerPos = context.player ? context.player->GetPosition() : target->position;

    // 1. Validate preconditions
    InteractionResult valid = InteractionValidator::Validate(target, playerPos, context);
    if (valid != InteractionResult::Success) {
        std::string feedback = "Cannot interact right now.";
        if (valid == InteractionResult::MissingRequirement) {
            feedback = "Locked - Key Required!";
        } else if (valid == InteractionResult::OutOfRange) {
            feedback = "Too far away.";
        } else if (valid == InteractionResult::CooldownActive) {
            feedback = "Please wait a moment.";
        } else if (valid == InteractionResult::Blocked) {
            feedback = "Action blocked.";
        }
        m_currentPrompt.SetFeedback(feedback, valid);

        InteractionEvent failEvent;
        failEvent.type = InteractionEventType::InteractionFailed;
        failEvent.interactableId = id;
        failEvent.interactionId = target->definition.id;
        failEvent.category = target->definition.category;
        failEvent.result = valid;
        failEvent.detail = feedback;
        DispatchEvent(failEvent);

        return valid;
    }

    // 2. Resolve domain pointers
    Door* door = (target->definition.category == InteractionCategory::Door) ? GetDoor(target->linkedEntityId) : nullptr;
    Mechanism* mechanism = (target->definition.category == InteractionCategory::Mechanism) ? GetMechanism(target->linkedEntityId) : nullptr;

    // 3. Dispatch execution
    InteractionEvent startEvent;
    startEvent.type = InteractionEventType::InteractionStarted;
    startEvent.interactableId = id;
    startEvent.interactionId = target->definition.id;
    startEvent.category = target->definition.category;
    DispatchEvent(startEvent);

    InteractionResult execResult = InteractionExecutor::Execute(target, context, door, mechanism);

    // 4. Update prompt and dispatch outcome events
    InteractionEvent outcomeEvent;
    outcomeEvent.interactableId = id;
    outcomeEvent.interactionId = target->definition.id;
    outcomeEvent.category = target->definition.category;
    outcomeEvent.result = execResult;

    if (execResult == InteractionResult::Success) {
        outcomeEvent.type = InteractionEventType::InteractionSucceeded;
        m_currentPrompt.SetFeedback("Success!", execResult, 1.0f);

        // Specific domain notifications
        if (target->definition.category == InteractionCategory::Door) {
            outcomeEvent.type = (door && door->IsOpen()) ? InteractionEventType::DoorOpened : InteractionEventType::DoorClosed;
        } else if (target->definition.category == InteractionCategory::Building) {
            outcomeEvent.type = (context.buildingManager && context.buildingManager->IsIndoors())
                                    ? InteractionEventType::BuildingEntered : InteractionEventType::BuildingExited;
        } else if (target->definition.category == InteractionCategory::Container) {
            outcomeEvent.type = InteractionEventType::ContainerOpened;
        } else if (target->definition.category == InteractionCategory::Collectible) {
            outcomeEvent.type = InteractionEventType::ItemCollected;
        } else if (target->definition.category == InteractionCategory::Mechanism) {
            outcomeEvent.type = (mechanism && mechanism->IsActive())
                                    ? InteractionEventType::MechanismActivated : InteractionEventType::MechanismDeactivated;
        }
    } else {
        outcomeEvent.type = InteractionEventType::InteractionFailed;
        m_currentPrompt.SetFeedback("Action failed.", execResult);
    }

    DispatchEvent(outcomeEvent);
    return execResult;
}

void InteractionManager::SyncWithWorld(InteractionContext& context) {
    if (!context.world) return;

    // 1. Sync LootContainers
    const auto& containers = context.world->GetLootContainers();
    for (const auto& c : containers) {
        if (!c) continue;
        std::string compId = "chest_" + c->GetId();
        bool alreadyRegistered = false;
        for (const auto& [id, existing] : m_interactables) {
            if (existing.definition.id == compId) {
                alreadyRegistered = true;
                break;
            }
        }
        if (!alreadyRegistered) {
            InteractableComponent comp;
            comp.position = c->GetPosition();
            comp.linkedEntityId = c->GetId();
            comp.definition.id = compId;
            comp.definition.displayName = c->GetName();
            comp.definition.verb = c->IsOpen() ? "Loot" : "Open";
            comp.definition.category = InteractionCategory::Container;
            comp.definition.range = 48.0f;
            comp.definition.priority = 12;
            comp.definition.requiredItemId = c->GetRequiredKeyId();
            RegisterInteractable(std::move(comp));
        }
    }

    // 2. Sync WorldItems (if manual pickup policy is active)
    if (m_pickupPolicy == PickupPolicy::Manual) {
        const auto& worldItems = context.world->GetWorldItems();
        for (const auto& wi : worldItems) {
            if (!wi || wi->IsCollected()) continue;
            std::string compId = "item_" + std::to_string(wi->GetId());
            bool alreadyRegistered = false;
            for (const auto& [id, existing] : m_interactables) {
                if (existing.definition.id == compId) {
                    alreadyRegistered = true;
                    break;
                }
            }
            if (!alreadyRegistered) {
                InteractableComponent comp;
                comp.position = wi->GetPosition();
                comp.linkedEntityId = std::to_string(wi->GetId());
                comp.definition.id = compId;
                comp.definition.displayName = wi->GetItem().definitionId;
                if (context.itemManager) {
                    const auto* def = context.itemManager->GetItem(wi->GetItem().definitionId);
                    if (def) comp.definition.displayName = def->displayName;
                }
                comp.definition.verb = "Collect";
                comp.definition.category = InteractionCategory::Collectible;
                comp.definition.range = wi->GetPickupRadius();
                comp.definition.priority = 5; // Items yield priority to chests/doors
                comp.definition.oneTimeOnly = true;
                RegisterInteractable(std::move(comp));
            }
        }
    }

    // 3. Sync Building entrances and exits
    if (context.buildingManager) {
        if (context.buildingManager->IsIndoors()) {
            // Interior exit door
            const auto* currBuilding = context.buildingManager->GetCurrentInteriorBuilding();
            if (currBuilding) {
                std::string compId = "bldg_exit_" + currBuilding->id;
                bool alreadyRegistered = false;
                for (const auto& [id, existing] : m_interactables) {
                    if (existing.definition.id == compId) {
                        alreadyRegistered = true;
                        break;
                    }
                }
                if (!alreadyRegistered) {
                    InteractableComponent comp;
                    comp.position = currBuilding->interiorExitPosition;
                    comp.linkedEntityId = currBuilding->id;
                    comp.definition.id = compId;
                    comp.definition.displayName = "Exit to " + context.buildingManager->GetVillageName();
                    comp.definition.verb = "Exit";
                    comp.definition.category = InteractionCategory::Building;
                    comp.definition.range = currBuilding->entranceRadius;
                    comp.definition.priority = 25;
                    RegisterInteractable(std::move(comp));
                }
            }
        } else {
            // Exterior building entrances
            const auto& buildings = context.buildingManager->GetBuildings();
            for (const auto& b : buildings) {
                if (!b.isEnterable) continue;
                std::string compId = "bldg_enter_" + b.id;
                bool alreadyRegistered = false;
                for (const auto& [id, existing] : m_interactables) {
                    if (existing.definition.id == compId) {
                        alreadyRegistered = true;
                        break;
                    }
                }
                if (!alreadyRegistered) {
                    InteractableComponent comp;
                    comp.position = b.entrancePosition;
                    comp.linkedEntityId = b.id;
                    comp.definition.id = compId;
                    comp.definition.displayName = b.name;
                    comp.definition.verb = "Enter";
                    comp.definition.category = InteractionCategory::Building;
                    comp.definition.range = b.entranceRadius;
                    comp.definition.priority = 25;
                    RegisterInteractable(std::move(comp));
                }
            }
        }
    }
}

} // namespace wildbound::interaction

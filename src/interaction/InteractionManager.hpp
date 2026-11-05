#pragma once

#include "InteractionTypes.hpp"
#include "InteractionDefinition.hpp"
#include "InteractableComponent.hpp"
#include "InteractionPrompt.hpp"
#include "InteractionContext.hpp"
#include "InteractionEvent.hpp"
#include "Door.hpp"
#include "Mechanism.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace wildbound::input {
    class InputManager;
}

namespace wildbound::interaction {

/**
 * Candidate interaction target with precomputed spatial metrics for deterministic selection.
 */
struct CandidateTarget {
    const InteractableComponent* component = nullptr;
    float distance = 0.0f;
    float facingAlignment = 0.0f; // Dot product with player's facing direction
    int priority = 0;
};

/**
 * Centralized, authoritative World Interaction Management System.
 * Coordinates nearby interactable discovery, deterministic candidate selection,
 * prompt UI generation, precondition validation, and domain execution.
 */
class InteractionManager {
public:
    InteractionManager();
    ~InteractionManager() = default;

    // Non-copyable, movable
    InteractionManager(const InteractionManager&) = delete;
    InteractionManager& operator=(const InteractionManager&) = delete;
    InteractionManager(InteractionManager&&) noexcept = default;
    InteractionManager& operator=(InteractionManager&&) noexcept = default;

    // --- Interactable Component Registration ---
    InteractableId RegisterInteractable(InteractableComponent component);
    void UnregisterInteractable(InteractableId id);
    InteractableComponent* GetInteractable(InteractableId id);
    const InteractableComponent* GetInteractable(InteractableId id) const;
    void ClearInteractables();

    // --- Domain Entity Registrations (Doors & Mechanisms) ---
    Door* RegisterDoor(Door door);
    Door* GetDoor(const std::string& id);
    const Door* GetDoor(const std::string& id) const;
    const std::vector<Door>& GetAllDoors() const { return m_doors; }
    void ClearDoors() { m_doors.clear(); }

    Mechanism* RegisterMechanism(Mechanism mechanism);
    Mechanism* GetMechanism(const std::string& id);
    const Mechanism* GetMechanism(const std::string& id) const;
    const std::vector<Mechanism>& GetAllMechanisms() const { return m_mechanisms; }
    void ClearMechanisms() { m_mechanisms.clear(); }

    // --- Policies & Config ---
    [[nodiscard]] PickupPolicy GetPickupPolicy() const noexcept { return m_pickupPolicy; }
    void SetPickupPolicy(PickupPolicy policy) noexcept { m_pickupPolicy = policy; }

    [[nodiscard]] float GetDefaultRange() const noexcept { return m_defaultRange; }
    void SetDefaultRange(float range) noexcept { m_defaultRange = range; }

    // --- Frame Lifecycle & Execution ---
    /**
     * Executes proximity discovery, candidate ranking, prompt updates, and input evaluation.
     */
    void Update(float dt, InteractionContext& context, const input::InputManager* inputMgr = nullptr);

    /**
     * Attempts interaction with the currently selected candidate target.
     */
    InteractionResult TryInteract(InteractionContext& context);

    /**
     * Directly interacts with a designated target.
     */
    InteractionResult InteractWith(InteractableId id, InteractionContext& context);

    // --- Queries & Diagnostics ---
    [[nodiscard]] const InteractableComponent* GetCurrentTarget() const noexcept { return m_currentTarget; }
    [[nodiscard]] const std::vector<CandidateTarget>& GetCurrentCandidates() const noexcept { return m_currentCandidates; }
    [[nodiscard]] const InteractionPrompt& GetCurrentPrompt() const noexcept { return m_currentPrompt; }
    [[nodiscard]] size_t GetInteractableCount() const noexcept { return m_interactables.size(); }
    const std::unordered_map<InteractableId, InteractableComponent>& GetAllInteractables() const noexcept { return m_interactables; }

    // --- Event Observers ---
    void AddListener(IInteractionListener* listener);
    void RemoveListener(IInteractionListener* listener);
    void DispatchEvent(const InteractionEvent& event);

    // --- Synchronization Helpers ---
    /**
     * Syncs world loot containers, ground items, and building entrances as registered interactables.
     */
    void SyncWithWorld(InteractionContext& context);

private:
    void UpdateDiscovery(const Vector2& playerPos, const Vector2& playerFacing);
    void UpdatePrompt(const input::InputManager* inputMgr);

    std::unordered_map<InteractableId, InteractableComponent> m_interactables;
    std::vector<Door> m_doors;
    std::vector<Mechanism> m_mechanisms;
    std::vector<IInteractionListener*> m_listeners;

    InteractableId m_nextInteractableId = 1;
    PickupPolicy m_pickupPolicy = PickupPolicy::Manual;
    float m_defaultRange = 52.0f;

    // Transient discovery cache
    const InteractableComponent* m_currentTarget = nullptr;
    std::vector<CandidateTarget> m_currentCandidates;
    InteractionPrompt m_currentPrompt;

    InteractableId m_lastTargetId = INVALID_INTERACTABLE_ID;
};

} // namespace wildbound::interaction


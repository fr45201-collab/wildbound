#pragma once

#include "CollisionTypes.hpp"
#include "CollisionFilter.hpp"
#include "Collider.hpp"
#include "CollisionEvent.hpp"
#include "OverlapDetector.hpp"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <memory>
#include <functional>
#include <cstdint>

namespace wildbound::physics {

/**
 * Key representing a unique, order-independent pair of colliders.
 * Sorted such that first <= second.
 */
struct ColliderPair {
    ColliderId first = INVALID_COLLIDER_ID;
    ColliderId second = INVALID_COLLIDER_ID;

    ColliderPair() = default;
    ColliderPair(ColliderId a, ColliderId b) noexcept {
        if (a < b) {
            first = a;
            second = b;
        } else {
            first = b;
            second = a;
        }
    }

    bool operator==(const ColliderPair& other) const noexcept {
        return first == other.first && second == other.second;
    }

    bool operator<(const ColliderPair& other) const noexcept {
        if (first != other.first) return first < other.first;
        return second < other.second;
    }
};

struct ColliderPairHash {
    size_t operator()(const ColliderPair& pair) const noexcept {
        return (std::hash<uint64_t>{}(pair.first) ^ (std::hash<uint64_t>{}(pair.second) << 1));
    }
};

/**
 * Deferred structural mutation for safe modification during simulation ticks.
 */
enum class MutationType {
    AddCollider,
    RemoveCollider,
    SetEnabled,
    SetLayerMask
};

struct DeferredMutation {
    MutationType type = MutationType::AddCollider;
    Collider collider;
    ColliderId id = INVALID_COLLIDER_ID;
    bool enabled = true;
    CollisionLayer layer = CollisionLayer::None;
    uint32_t mask = 0u;
};

/**
 * Centralized, authoritative collision and overlap management subsystem.
 * Coordinates spatial registration, bilateral mask filtering, narrow-phase overlap detection,
 * stateful trigger tracking, deferred mutation queuing, and deterministic event dispatching.
 */
class CollisionManager {
public:
    using CallbackId = uint32_t;

    CollisionManager();
    ~CollisionManager() = default;

    // Non-copyable, movable
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;
    CollisionManager(CollisionManager&&) noexcept = default;
    CollisionManager& operator=(CollisionManager&&) noexcept = default;

    // --- Collider Registration & Lifecycle ---
    ColliderId RegisterCollider(const Collider& collider);
    ColliderId RegisterStaticBox(CollisionLayer layer, const Rectangle& bounds, const std::string& tag = "static_world");
    void UnregisterCollider(ColliderId id);
    void UnregisterEntityColliders(EntityId entityId);

    const Collider* GetCollider(ColliderId id) const;
    Collider* GetColliderMutable(ColliderId id);

    void SetColliderEnabled(ColliderId id, bool enabled);
    void SetColliderTransform(ColliderId id, Vector2 entityPos);
    void SetColliderLayerAndMask(ColliderId id, CollisionLayer layer, uint32_t mask);

    // --- Event Observers & Callbacks ---
    void AddListener(ICollisionListener* listener);
    void RemoveListener(ICollisionListener* listener);

    CallbackId RegisterCallback(CollisionCallback callback);
    void UnregisterCallback(CallbackId id);

    // --- Simulation Tick & Update ---
    /**
     * Executes the deterministic collision and trigger update sequence.
     */
    void Update(float dt);

    // --- Spatial Queries ---
    std::vector<ColliderId> QueryAABB(const Rectangle& box, uint32_t mask = 0xFFFFFFFFu) const;
    bool HasOverlap(const Rectangle& box, uint32_t mask = 0xFFFFFFFFu) const;
    bool IsOverlapping(ColliderId a, ColliderId b) const;
    std::vector<EntityId> GetOverlappingEntities(ColliderId id) const;
    std::vector<ColliderId> GetCollidersForEntity(EntityId entityId) const;

    // Clear all simulation state
    void Clear();

    // --- Diagnostic Metrics ---
    size_t GetTotalColliderCount() const noexcept { return m_colliders.size(); }
    size_t GetActiveColliderCount() const noexcept;
    size_t GetActiveOverlapCount() const noexcept { return m_currentOverlaps.size(); }
    size_t GetEventsDispatchedLastTick() const noexcept { return m_eventsDispatchedLastTick; }
    size_t GetPairsFilteredLastTick() const noexcept { return m_pairsFilteredLastTick; }
    size_t GetPairsTestedLastTick() const noexcept { return m_pairsTestedLastTick; }

    const std::unordered_map<ColliderId, Collider>& GetAllColliders() const noexcept { return m_colliders; }
    const std::map<ColliderPair, Rectangle>& GetCurrentOverlaps() const noexcept { return m_currentOverlaps; }

private:
    void ProcessDeferredMutations();
    void ApplyMutation(const DeferredMutation& mutation);

    ColliderId m_nextColliderId = 1;
    CallbackId m_nextCallbackId = 1;
    bool m_inUpdateTick = false;

    // Authoritative Storage
    std::unordered_map<ColliderId, Collider> m_colliders;
    std::vector<ColliderId> m_staticColliderIds;
    std::vector<ColliderId> m_dynamicColliderIds;

    // Overlap State tracking (Deterministic std::map sorted by ColliderPair)
    std::map<ColliderPair, Rectangle> m_previousOverlaps;
    std::map<ColliderPair, Rectangle> m_currentOverlaps;

    // Deferred mutations queue
    std::vector<DeferredMutation> m_deferredMutations;

    // Listeners and callbacks
    std::vector<ICollisionListener*> m_listeners;
    std::unordered_map<CallbackId, CollisionCallback> m_callbacks;

    // Metrics
    size_t m_eventsDispatchedLastTick = 0;
    size_t m_pairsFilteredLastTick = 0;
    size_t m_pairsTestedLastTick = 0;
};

} // namespace wildbound::physics


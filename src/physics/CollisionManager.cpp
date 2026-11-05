#include "CollisionManager.hpp"
#include "../core/Logger.hpp"
#include <algorithm>

namespace wildbound::physics {

CollisionManager::CollisionManager() {
    m_listeners.reserve(16);
    m_deferredMutations.reserve(32);
}

ColliderId CollisionManager::RegisterCollider(const Collider& collider) {
    Collider col = collider;
    if (col.GetId() == INVALID_COLLIDER_ID) {
        col.SetId(m_nextColliderId++);
    } else if (col.GetId() >= m_nextColliderId) {
        m_nextColliderId = col.GetId() + 1;
    }

    if (m_inUpdateTick) {
        DeferredMutation mutation;
        mutation.type = MutationType::AddCollider;
        mutation.collider = col;
        mutation.id = col.GetId();
        m_deferredMutations.push_back(mutation);
        return col.GetId();
    }

    ColliderId id = col.GetId();
    bool isStatic = col.IsStatic();
    m_colliders[id] = col;

    if (isStatic) {
        m_staticColliderIds.push_back(id);
    } else {
        m_dynamicColliderIds.push_back(id);
    }

    return id;
}

ColliderId CollisionManager::RegisterStaticBox(CollisionLayer layer, const Rectangle& bounds, const std::string& tag) {
    Collider col = Collider::CreateStatic(m_nextColliderId++, layer, bounds, tag);
    return RegisterCollider(col);
}

void CollisionManager::UnregisterCollider(ColliderId id) {
    if (id == INVALID_COLLIDER_ID) return;

    if (m_inUpdateTick) {
        DeferredMutation mutation;
        mutation.type = MutationType::RemoveCollider;
        mutation.id = id;
        m_deferredMutations.push_back(mutation);
        return;
    }

    auto it = m_colliders.find(id);
    if (it != m_colliders.end()) {
        bool isStatic = it->second.IsStatic();
        m_colliders.erase(it);

        if (isStatic) {
            auto sIt = std::find(m_staticColliderIds.begin(), m_staticColliderIds.end(), id);
            if (sIt != m_staticColliderIds.end()) m_staticColliderIds.erase(sIt);
        } else {
            auto dIt = std::find(m_dynamicColliderIds.begin(), m_dynamicColliderIds.end(), id);
            if (dIt != m_dynamicColliderIds.end()) m_dynamicColliderIds.erase(dIt);
        }

        // Clean up from active overlap tracking
        for (auto pIt = m_previousOverlaps.begin(); pIt != m_previousOverlaps.end(); ) {
            if (pIt->first.first == id || pIt->first.second == id) {
                pIt = m_previousOverlaps.erase(pIt);
            } else {
                ++pIt;
            }
        }
        for (auto cIt = m_currentOverlaps.begin(); cIt != m_currentOverlaps.end(); ) {
            if (cIt->first.first == id || cIt->first.second == id) {
                cIt = m_currentOverlaps.erase(cIt);
            } else {
                ++cIt;
            }
        }
    }
}

void CollisionManager::UnregisterEntityColliders(EntityId entityId) {
    if (entityId == INVALID_ENTITY_ID) return;

    std::vector<ColliderId> toRemove;
    for (const auto& [id, col] : m_colliders) {
        if (col.GetEntityId() == entityId) {
            toRemove.push_back(id);
        }
    }

    for (ColliderId id : toRemove) {
        UnregisterCollider(id);
    }
}

const Collider* CollisionManager::GetCollider(ColliderId id) const {
    auto it = m_colliders.find(id);
    return (it != m_colliders.end()) ? &it->second : nullptr;
}

Collider* CollisionManager::GetColliderMutable(ColliderId id) {
    auto it = m_colliders.find(id);
    return (it != m_colliders.end()) ? &it->second : nullptr;
}

void CollisionManager::SetColliderEnabled(ColliderId id, bool enabled) {
    if (m_inUpdateTick) {
        DeferredMutation mutation;
        mutation.type = MutationType::SetEnabled;
        mutation.id = id;
        mutation.enabled = enabled;
        m_deferredMutations.push_back(mutation);
        return;
    }

    auto* col = GetColliderMutable(id);
    if (col) {
        col->SetEnabled(enabled);
    }
}

void CollisionManager::SetColliderTransform(ColliderId id, Vector2 entityPos) {
    auto* col = GetColliderMutable(id);
    if (col && !col->IsStatic()) {
        col->UpdateWorldBounds(entityPos);
    }
}

void CollisionManager::SetColliderLayerAndMask(ColliderId id, CollisionLayer layer, uint32_t mask) {
    if (m_inUpdateTick) {
        DeferredMutation mutation;
        mutation.type = MutationType::SetLayerMask;
        mutation.id = id;
        mutation.layer = layer;
        mutation.mask = mask;
        m_deferredMutations.push_back(mutation);
        return;
    }

    auto* col = GetColliderMutable(id);
    if (col) {
        col->SetLayer(layer);
        col->SetMask(mask);
    }
}

void CollisionManager::AddListener(ICollisionListener* listener) {
    if (!listener) return;
    auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
    if (it == m_listeners.end()) {
        m_listeners.push_back(listener);
    }
}

void CollisionManager::RemoveListener(ICollisionListener* listener) {
    if (!listener) return;
    auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
    if (it != m_listeners.end()) {
        m_listeners.erase(it);
    }
}

CollisionManager::CallbackId CollisionManager::RegisterCallback(CollisionCallback callback) {
    CallbackId id = m_nextCallbackId++;
    m_callbacks[id] = std::move(callback);
    return id;
}

void CollisionManager::UnregisterCallback(CallbackId id) {
    m_callbacks.erase(id);
}

void CollisionManager::ApplyMutation(const DeferredMutation& mutation) {
    switch (mutation.type) {
        case MutationType::AddCollider: {
            ColliderId id = mutation.collider.GetId();
            bool isStatic = mutation.collider.IsStatic();
            m_colliders[id] = mutation.collider;
            if (isStatic) {
                m_staticColliderIds.push_back(id);
            } else {
                m_dynamicColliderIds.push_back(id);
            }
            break;
        }
        case MutationType::RemoveCollider: {
            UnregisterCollider(mutation.id);
            break;
        }
        case MutationType::SetEnabled: {
            auto* col = GetColliderMutable(mutation.id);
            if (col) col->SetEnabled(mutation.enabled);
            break;
        }
        case MutationType::SetLayerMask: {
            auto* col = GetColliderMutable(mutation.id);
            if (col) {
                col->SetLayer(mutation.layer);
                col->SetMask(mutation.mask);
            }
            break;
        }
    }
}

void CollisionManager::ProcessDeferredMutations() {
    if (m_deferredMutations.empty()) return;

    // Move to local queue so mutations triggered during processing are safe
    std::vector<DeferredMutation> queue;
    queue.swap(m_deferredMutations);

    for (const auto& mutation : queue) {
        ApplyMutation(mutation);
    }
}

void CollisionManager::Update(float /*dt*/) {
    // 1. Process mutations queued before tick
    ProcessDeferredMutations();

    m_inUpdateTick = true;
    m_currentOverlaps.clear();
    m_eventsDispatchedLastTick = 0;
    m_pairsFilteredLastTick = 0;
    m_pairsTestedLastTick = 0;

    // 2. Generate Candidate Pairs (Dynamic vs Static, Dynamic vs Dynamic)
    std::vector<ColliderPair> candidatePairs;
    candidatePairs.reserve((m_dynamicColliderIds.size() * m_staticColliderIds.size()) +
                           (m_dynamicColliderIds.size() * (m_dynamicColliderIds.size() > 0 ? m_dynamicColliderIds.size() - 1 : 0) / 2));

    // Dynamic vs Static
    for (ColliderId dynId : m_dynamicColliderIds) {
        for (ColliderId statId : m_staticColliderIds) {
            candidatePairs.emplace_back(dynId, statId);
        }
    }

    // Dynamic vs Dynamic
    for (size_t i = 0; i < m_dynamicColliderIds.size(); ++i) {
        for (size_t j = i + 1; j < m_dynamicColliderIds.size(); ++j) {
            candidatePairs.emplace_back(m_dynamicColliderIds[i], m_dynamicColliderIds[j]);
        }
    }

    // 3. Sort Candidate Pairs deterministically
    std::sort(candidatePairs.begin(), candidatePairs.end());

    // 4. Bilateral Filtering & Narrow-Phase Overlap Detection
    for (const auto& pair : candidatePairs) {
        auto itA = m_colliders.find(pair.first);
        auto itB = m_colliders.find(pair.second);
        if (itA == m_colliders.end() || itB == m_colliders.end()) continue;

        const auto& colA = itA->second;
        const auto& colB = itB->second;

        if (!CollisionFilter::ShouldTestPair(
                colA.IsEnabled(), colA.GetEntityId(), colA.GetLayer(), colA.GetMask(),
                colB.IsEnabled(), colB.GetEntityId(), colB.GetLayer(), colB.GetMask()))
        {
            m_pairsFilteredLastTick++;
            continue;
        }

        m_pairsTestedLastTick++;

        Rectangle intersection = OverlapDetector::ComputeIntersection(colA.GetWorldBounds(), colB.GetWorldBounds());
        if (intersection.width > 0.0f && intersection.height > 0.0f) {
            m_currentOverlaps[pair] = intersection;
        }
    }

    // 5. Generate Collision & Trigger Events
    std::vector<CollisionEvent> eventsToDispatch;
    eventsToDispatch.reserve(m_currentOverlaps.size() + m_previousOverlaps.size());

    // Enter & Stay Events
    for (const auto& [pair, rect] : m_currentOverlaps) {
        auto itA = m_colliders.find(pair.first);
        auto itB = m_colliders.find(pair.second);
        if (itA == m_colliders.end() || itB == m_colliders.end()) continue;

        bool isTrigger = itA->second.IsTrigger() || itB->second.IsTrigger();
        bool wasOverlapping = m_previousOverlaps.find(pair) != m_previousOverlaps.end();

        CollisionEvent ev;
        ev.colliderIdA = pair.first;
        ev.colliderIdB = pair.second;
        ev.entityIdA = itA->second.GetEntityId();
        ev.entityIdB = itB->second.GetEntityId();
        ev.layerA = itA->second.GetLayer();
        ev.layerB = itB->second.GetLayer();
        ev.intersection = rect;
        ev.isTrigger = isTrigger;

        if (wasOverlapping) {
            ev.type = isTrigger ? CollisionEventType::TriggerStay : CollisionEventType::CollisionStay;
        } else {
            ev.type = isTrigger ? CollisionEventType::TriggerEnter : CollisionEventType::CollisionEnter;
        }

        eventsToDispatch.push_back(ev);
    }

    // Exit Events
    for (const auto& [pair, rect] : m_previousOverlaps) {
        if (m_currentOverlaps.find(pair) == m_currentOverlaps.end()) {
            auto itA = m_colliders.find(pair.first);
            auto itB = m_colliders.find(pair.second);

            // Even if one collider was deleted or disabled, generate Exit event using last known state
            bool isTrigger = false;
            EntityId entA = INVALID_ENTITY_ID;
            EntityId entB = INVALID_ENTITY_ID;
            CollisionLayer layA = CollisionLayer::None;
            CollisionLayer layB = CollisionLayer::None;

            if (itA != m_colliders.end()) {
                isTrigger = isTrigger || itA->second.IsTrigger();
                entA = itA->second.GetEntityId();
                layA = itA->second.GetLayer();
            }
            if (itB != m_colliders.end()) {
                isTrigger = isTrigger || itB->second.IsTrigger();
                entB = itB->second.GetEntityId();
                layB = itB->second.GetLayer();
            }

            CollisionEvent ev;
            ev.type = isTrigger ? CollisionEventType::TriggerExit : CollisionEventType::CollisionExit;
            ev.colliderIdA = pair.first;
            ev.colliderIdB = pair.second;
            ev.entityIdA = entA;
            ev.entityIdB = entB;
            ev.layerA = layA;
            ev.layerB = layB;
            ev.intersection = Rectangle{ 0.0f, 0.0f, 0.0f, 0.0f };
            ev.isTrigger = isTrigger;

            eventsToDispatch.push_back(ev);
        }
    }

    // 6. Sort Events deterministically (by type, then colliderIdA, then colliderIdB)
    std::sort(eventsToDispatch.begin(), eventsToDispatch.end(), [](const CollisionEvent& a, const CollisionEvent& b) {
        if (static_cast<uint8_t>(a.type) != static_cast<uint8_t>(b.type)) {
            return static_cast<uint8_t>(a.type) < static_cast<uint8_t>(b.type);
        }
        if (a.colliderIdA != b.colliderIdA) return a.colliderIdA < b.colliderIdA;
        return a.colliderIdB < b.colliderIdB;
    });

    // 7. Dispatch Events
    for (const auto& ev : eventsToDispatch) {
        for (auto* listener : m_listeners) {
            if (listener) listener->OnCollisionEvent(ev);
        }
        for (const auto& [cbId, callback] : m_callbacks) {
            if (callback) callback(ev);
        }
        m_eventsDispatchedLastTick++;
    }

    m_inUpdateTick = false;

    // 8. Process mutations queued during event callbacks
    ProcessDeferredMutations();

    // 9. Finalize active overlap table for next tick
    m_previousOverlaps = m_currentOverlaps;
}

std::vector<ColliderId> CollisionManager::QueryAABB(const Rectangle& box, uint32_t mask) const {
    std::vector<ColliderId> results;
    for (const auto& [id, col] : m_colliders) {
        if (!col.IsEnabled()) continue;
        if ((mask & static_cast<uint32_t>(col.GetLayer())) == 0) continue;

        if (OverlapDetector::Intersects(box, col.GetWorldBounds())) {
            results.push_back(id);
        }
    }
    return results;
}

bool CollisionManager::HasOverlap(const Rectangle& box, uint32_t mask) const {
    for (const auto& [id, col] : m_colliders) {
        if (!col.IsEnabled()) continue;
        if ((mask & static_cast<uint32_t>(col.GetLayer())) == 0) continue;

        if (OverlapDetector::Intersects(box, col.GetWorldBounds())) {
            return true;
        }
    }
    return false;
}

bool CollisionManager::IsOverlapping(ColliderId a, ColliderId b) const {
    ColliderPair pair(a, b);
    return m_currentOverlaps.find(pair) != m_currentOverlaps.end();
}

std::vector<EntityId> CollisionManager::GetOverlappingEntities(ColliderId id) const {
    std::unordered_set<EntityId> entities;
    for (const auto& [pair, rect] : m_currentOverlaps) {
        if (pair.first == id) {
            auto it = m_colliders.find(pair.second);
            if (it != m_colliders.end() && it->second.GetEntityId() != INVALID_ENTITY_ID) {
                entities.insert(it->second.GetEntityId());
            }
        } else if (pair.second == id) {
            auto it = m_colliders.find(pair.first);
            if (it != m_colliders.end() && it->second.GetEntityId() != INVALID_ENTITY_ID) {
                entities.insert(it->second.GetEntityId());
            }
        }
    }
    return std::vector<EntityId>(entities.begin(), entities.end());
}

std::vector<ColliderId> CollisionManager::GetCollidersForEntity(EntityId entityId) const {
    std::vector<ColliderId> results;
    for (const auto& [id, col] : m_colliders) {
        if (col.GetEntityId() == entityId) {
            results.push_back(id);
        }
    }
    return results;
}

size_t CollisionManager::GetActiveColliderCount() const noexcept {
    size_t count = 0;
    for (const auto& [id, col] : m_colliders) {
        if (col.IsEnabled()) count++;
    }
    return count;
}

void CollisionManager::Clear() {
    m_colliders.clear();
    m_staticColliderIds.clear();
    m_dynamicColliderIds.clear();
    m_previousOverlaps.clear();
    m_currentOverlaps.clear();
    m_deferredMutations.clear();
    m_listeners.clear();
    m_callbacks.clear();
    m_eventsDispatchedLastTick = 0;
    m_pairsFilteredLastTick = 0;
    m_pairsTestedLastTick = 0;
}

} // namespace wildbound::physics


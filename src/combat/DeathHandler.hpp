#pragma once

#include "CombatTypes.hpp"
#include "CombatEvents.hpp"
#include <unordered_set>
#include <vector>

namespace wildbound::combat {

class DeathHandler {
public:
    DeathHandler() = default;

    bool ProcessDeath(CombatantId victimId, CombatantId killerId, Vector2 deathPos, CombatEventDispatcher* dispatcher = nullptr) {
        if (m_deadEntities.find(victimId) != m_deadEntities.end()) {
            // Already dead - prevent repeated death triggers!
            return false;
        }

        m_deadEntities.insert(victimId);

        DeathEvent evt;
        evt.victimId = victimId;
        evt.killerId = killerId;
        evt.deathPosition = deathPos;
        evt.timestamp = static_cast<float>(GetTime());

        if (dispatcher) {
            dispatcher->Dispatch(evt);
        }

        return true;
    }

    bool IsDead(CombatantId id) const {
        return m_deadEntities.find(id) != m_deadEntities.end();
    }

    void Reset() {
        m_deadEntities.clear();
    }

    void Remove(CombatantId id) {
        m_deadEntities.erase(id);
    }

private:
    std::unordered_set<CombatantId> m_deadEntities;
};

} // namespace wildbound::combat


#include "ReplicationManager.hpp"
#include <algorithm>
#include <cmath>

namespace wildbound::network {

ReplicationManager::ReplicationManager(NetworkRole role)
    : m_role(role) {}

bool ReplicationManager::Initialize(const std::string& configPath) {
    if (!m_config.LoadFromFile(configPath)) {
        // Fallback to secondary location
        if (!m_config.LoadFromFile("data/networking/network_replication.json")) {
            m_config = ReplicationConfiguration::GetDefault();
        }
    }

    m_snapshotBuffer = SnapshotBuffer(m_config.maxSnapshotBuffer);
    m_interpolator = ReplicationInterpolator(250.0f);
    m_eventDeduplicator = EventDeduplicator(512);

    Reset();
    return true;
}

void ReplicationManager::Reset() {
    m_registry.Clear();
    m_snapshotBuffer.Clear();
    m_eventDeduplicator.Clear();
    m_authoritativeStates.clear();
    m_baselineStates.clear();

    m_serverTick = 0;
    m_serverTime = 0.0f;
    m_clientTime = 0.0f;
    m_snapshotTimer = 0.0f;
    m_nextEventId = 1000;

    m_snapshotsSent = 0;
    m_snapshotsReceived = 0;
    m_deltaSnapshotsSent = 0;
    m_deltaSnapshotsReceived = 0;
    m_combatEventsDispatched = 0;
    m_pickupProcessed = 0;
    m_pickupSuccess = 0;
    m_pickupRejected = 0;
    m_lastInterpolationAlpha = 0.0f;
}

void ReplicationManager::Update(float dt) {
    if (m_role == NetworkRole::Server || m_role == NetworkRole::ListenServer) {
        m_serverTick++;
        m_serverTime += dt;
        m_snapshotTimer += dt;
    } else {
        m_clientTime += dt;
    }
}

// ----------------------------------------------------------------------------
// Server Authority Implementations
// ----------------------------------------------------------------------------

NetworkEntityIdentity ReplicationManager::RegisterPlayer(uint32_t localId, float x, float y) {
    auto identity = m_registry.RegisterEntity(NetworkEntityType::Player, localId, static_cast<uint32_t>(m_serverTick));

    EntitySnapshotRecord record{};
    record.identity = identity;
    record.type = NetworkEntityType::Player;
    record.dirtyMask = EntityDirtyMask::All;
    record.posX = x;
    record.posY = y;
    record.health = 100.0f;
    record.maxHealth = 100.0f;
    record.SetAlive(true);

    m_authoritativeStates[identity.ToHandle()] = record;
    return identity;
}

NetworkEntityIdentity ReplicationManager::RegisterEnemy(uint32_t localId, float x, float y, float health, uint8_t aiState) {
    auto identity = m_registry.RegisterEntity(NetworkEntityType::Enemy, localId, static_cast<uint32_t>(m_serverTick));

    EntitySnapshotRecord record{};
    record.identity = identity;
    record.type = NetworkEntityType::Enemy;
    record.dirtyMask = EntityDirtyMask::All;
    record.posX = x;
    record.posY = y;
    record.health = health;
    record.maxHealth = health;
    record.animState = aiState;
    record.SetAlive(true);

    m_authoritativeStates[identity.ToHandle()] = record;
    return identity;
}

NetworkEntityIdentity ReplicationManager::RegisterLoot(uint32_t localId, uint32_t itemDefId, float x, float y, uint32_t quantity) {
    auto identity = m_registry.RegisterEntity(NetworkEntityType::Loot, localId, static_cast<uint32_t>(m_serverTick));

    EntitySnapshotRecord record{};
    record.identity = identity;
    record.type = NetworkEntityType::Loot;
    record.dirtyMask = EntityDirtyMask::All;
    record.posX = x;
    record.posY = y;
    record.customData1 = itemDefId;
    record.customData2 = quantity;
    record.SetAlive(true); // IsAvailable

    m_authoritativeStates[identity.ToHandle()] = record;
    return identity;
}

NetworkEntityIdentity ReplicationManager::RegisterEnvironment(uint32_t localId, float x, float y, uint16_t stateId) {
    auto identity = m_registry.RegisterEntity(NetworkEntityType::EnvironmentalObject, localId, static_cast<uint32_t>(m_serverTick));

    EntitySnapshotRecord record{};
    record.identity = identity;
    record.type = NetworkEntityType::EnvironmentalObject;
    record.dirtyMask = EntityDirtyMask::All;
    record.posX = x;
    record.posY = y;
    record.customData1 = stateId;
    record.SetAlive(true); // Active

    m_authoritativeStates[identity.ToHandle()] = record;
    return identity;
}

bool ReplicationManager::DestroyEntity(NetworkEntityIdentity identity, uint8_t reasonCode) {
    (void)reasonCode;
    if (!m_registry.UnregisterEntity(identity)) {
        return false;
    }
    m_authoritativeStates.erase(identity.ToHandle());
    m_baselineStates.erase(identity.ToHandle());
    return true;
}

bool ReplicationManager::ServerProcessInput(NetworkEntityIdentity playerId, const MsgPlayerInput& input, float dt) {
    if (!m_registry.IsEntityValid(playerId)) {
        return false;
    }

    auto it = m_authoritativeStates.find(playerId.ToHandle());
    if (it == m_authoritativeStates.end() || it->second.type != NetworkEntityType::Player) {
        return false;
    }

    auto& state = it->second;
    if (!state.IsAlive()) {
        return false;
    }

    // Directional normalization & validation
    float dirX = input.dirX;
    float dirY = input.dirY;
    const float lenSq = dirX * dirX + dirY * dirY;
    if (lenSq > 1.0f) {
        const float invLen = 1.0f / std::sqrt(lenSq);
        dirX *= invLen;
        dirY *= invLen;
    }

    // Speed calculation
    const bool isSprinting = (input.actionFlags & 0x01) != 0;
    const float baseSpeed = 160.0f; // units / sec
    const float speed = isSprinting ? (baseSpeed * 1.5f) : baseSpeed;

    const float velX = dirX * speed;
    const float velY = dirY * speed;

    state.posX += velX * dt;
    state.posY += velY * dt;
    state.velX = velX;
    state.velY = velY;
    state.facingAngle = input.facingAngle;

    state.SetMoving(dirX != 0.0f || dirY != 0.0f);
    state.SetRunning(state.IsMoving() && isSprinting);

    state.dirtyMask = state.dirtyMask |
                      EntityDirtyMask::DirtyPosition |
                      EntityDirtyMask::DirtyVelocity |
                      EntityDirtyMask::DirtyFacing |
                      EntityDirtyMask::DirtyStatusFlags;

    return true;
}

bool ReplicationManager::ServerProcessAttack(NetworkEntityIdentity attackerId, const MsgAttackRequest& request) {
    if (!m_registry.IsEntityValid(attackerId)) {
        return false;
    }

    auto itAttacker = m_authoritativeStates.find(attackerId.ToHandle());
    if (itAttacker == m_authoritativeStates.end() || !itAttacker->second.IsAlive()) {
        return false;
    }

    const auto& attacker = itAttacker->second;
    const float attackRange = (request.weaponSlot == 0) ? 80.0f : 350.0f;
    const float baseDamage = (request.weaponSlot == 0) ? 25.0f : 45.0f;

    // Search for closest target in attack cone
    NetworkEntityIdentity targetId{};
    float closestDistSq = attackRange * attackRange;

    for (auto& [handle, target] : m_authoritativeStates) {
        if (target.identity == attackerId || !target.IsAlive()) {
            continue;
        }

        const float dx = target.posX - attacker.posX;
        const float dy = target.posY - attacker.posY;
        const float distSq = dx * dx + dy * dy;

        if (distSq <= closestDistSq) {
            closestDistSq = distSq;
            targetId = target.identity;
        }
    }

    if (!targetId.IsValid()) {
        return false; // Attack missed
    }

    auto& target = m_authoritativeStates[targetId.ToHandle()];
    target.health = std::max(0.0f, target.health - baseDamage);
    const bool isElim = (target.health <= 0.0f);
    if (isElim) {
        target.SetAlive(false);
    }
    target.dirtyMask = target.dirtyMask | EntityDirtyMask::DirtyHealthArmor | EntityDirtyMask::DirtyStatusFlags;

    CombatEvent evt{
        .eventId = ++m_nextEventId,
        .sourceEntity = attackerId,
        .targetEntity = targetId,
        .combatAction = (request.weaponSlot == 0) ? 1u : 2u,
        .damageAmount = baseDamage,
        .remainingHealth = target.health,
        .isCritical = false,
        .isElimination = isElim,
        .serverTick = static_cast<uint32_t>(m_serverTick)
    };

    m_combatEventsDispatched++;
    if (m_onCombatEvent) {
        m_onCombatEvent(evt);
    }

    return true;
}

bool ReplicationManager::ServerProcessPickup(const PickupRequestEvent& request) {
    m_pickupProcessed++;

    if (!m_registry.IsEntityValid(request.targetLootEntity) || !m_registry.IsEntityValid(request.requestingPlayer)) {
        m_pickupRejected++;
        return false;
    }

    auto itLoot = m_authoritativeStates.find(request.targetLootEntity.ToHandle());
    auto itPlayer = m_authoritativeStates.find(request.requestingPlayer.ToHandle());

    if (itLoot == m_authoritativeStates.end() || itPlayer == m_authoritativeStates.end()) {
        m_pickupRejected++;
        return false;
    }

    auto& loot = itLoot->second;
    const auto& player = itPlayer->second;

    if (!loot.IsAlive() || !player.IsAlive() || loot.type != NetworkEntityType::Loot) {
        m_pickupRejected++;
        return false;
    }

    // Distance check
    const float dx = loot.posX - player.posX;
    const float dy = loot.posY - player.posY;
    const float maxPickupDist = 120.0f;
    if (dx * dx + dy * dy > maxPickupDist * maxPickupDist) {
        m_pickupRejected++;
        return false;
    }

    // Atomic collection
    loot.SetAlive(false);
    const uint32_t itemDefId = loot.customData1;
    const uint32_t quantity = loot.customData2;

    DestroyEntity(request.targetLootEntity, 1);

    m_pickupSuccess++;

    PickupResultEvent result{
        .eventId = ++m_nextEventId,
        .targetLootEntity = request.targetLootEntity,
        .recipientPlayer = request.requestingPlayer,
        .itemDefinitionId = itemDefId,
        .quantity = quantity,
        .success = true
    };

    if (m_onPickupResult) {
        m_onPickupResult(result);
    }

    return true;
}

bool ReplicationManager::ServerUpdateEnvironment(NetworkEntityIdentity entityId, uint16_t stateId, bool active, bool destroyed) {
    if (!m_registry.IsEntityValid(entityId)) {
        return false;
    }

    auto it = m_authoritativeStates.find(entityId.ToHandle());
    if (it == m_authoritativeStates.end() || it->second.type != NetworkEntityType::EnvironmentalObject) {
        return false;
    }

    auto& state = it->second;
    state.customData1 = stateId;
    state.SetAlive(active && !destroyed);
    state.dirtyMask = state.dirtyMask | EntityDirtyMask::DirtyStatusFlags | EntityDirtyMask::DirtyWeapon;

    EnvironmentalStateEvent evt{
        .eventId = ++m_nextEventId,
        .entityId = entityId,
        .stateId = stateId,
        .active = active,
        .destroyed = destroyed,
        .stateVersion = state.customData2 + 1,
        .serverTick = static_cast<uint32_t>(m_serverTick)
    };
    state.customData2 = evt.stateVersion;

    if (m_onEnvironmentalEvent) {
        m_onEnvironmentalEvent(evt);
    }
    return true;
}

void ReplicationManager::ServerUpdateStorm(uint8_t phase, float cx, float cy, float radius, float targetRadius, float dps) {
    StormStateEvent evt{
        .currentPhase = phase,
        .centerX = cx,
        .centerY = cy,
        .currentRadius = radius,
        .targetRadius = targetRadius,
        .dps = dps,
        .serverTick = static_cast<uint32_t>(m_serverTick)
    };

    if (m_onStormEvent) {
        m_onStormEvent(evt);
    }
}

WorldSnapshot ReplicationManager::BuildSnapshot(bool forceFull) {
    WorldSnapshot snapshot;
    snapshot.serverTick = m_serverTick;
    snapshot.timestamp = m_serverTime;
    snapshot.isDelta = (!forceFull && m_config.enableDeltaSnapshots && !m_baselineStates.empty());
    snapshot.baselineTick = snapshot.isDelta ? (m_serverTick - 1) : 0;

    snapshot.entities.reserve(m_authoritativeStates.size());

    for (auto& [handle, current] : m_authoritativeStates) {
        if (snapshot.isDelta) {
            auto itBase = m_baselineStates.find(handle);
            if (itBase != m_baselineStates.end()) {
                EntityDirtyMask mask = EntityDirtyMask::Clean;
                const auto& base = itBase->second;

                if (current.posX != base.posX || current.posY != base.posY) {
                    mask = mask | EntityDirtyMask::DirtyPosition;
                }
                if (current.velX != base.velX || current.velY != base.velY) {
                    mask = mask | EntityDirtyMask::DirtyVelocity;
                }
                if (current.facingAngle != base.facingAngle) {
                    mask = mask | EntityDirtyMask::DirtyFacing;
                }
                if (current.health != base.health || current.maxHealth != base.maxHealth || current.armor != base.armor) {
                    mask = mask | EntityDirtyMask::DirtyHealthArmor;
                }
                if (current.animState != base.animState) {
                    mask = mask | EntityDirtyMask::DirtyAnimState;
                }
                if (current.statusFlags != base.statusFlags) {
                    mask = mask | EntityDirtyMask::DirtyStatusFlags;
                }
                if (current.customData1 != base.customData1 || current.customData2 != base.customData2) {
                    mask = mask | EntityDirtyMask::DirtyWeapon;
                }

                current.dirtyMask = mask;
            } else {
                current.dirtyMask = EntityDirtyMask::All;
            }
        } else {
            current.dirtyMask = EntityDirtyMask::All;
        }

        snapshot.entities.push_back(current);
    }

    // Update baseline
    m_baselineStates = m_authoritativeStates;

    m_snapshotsSent++;
    if (snapshot.isDelta) {
        m_deltaSnapshotsSent++;
    }

    return snapshot;
}

// ----------------------------------------------------------------------------
// Client Authority Implementations
// ----------------------------------------------------------------------------

bool ReplicationManager::ClientReceiveSnapshot(const WorldSnapshot& snapshot) {
    if (!m_snapshotBuffer.PushSnapshot(snapshot)) {
        return false;
    }

    m_snapshotsReceived++;
    if (snapshot.isDelta) {
        m_deltaSnapshotsReceived++;
    }

    // Ensure entities present in snapshot are registered in client registry
    for (const auto& ent : snapshot.entities) {
        if (!m_registry.IsEntityValid(ent.identity)) {
            m_registry.RegisterRemoteEntity(ent.identity, ent.type, 0, static_cast<uint32_t>(snapshot.serverTick));
        }
    }

    return true;
}

bool ReplicationManager::ClientReceiveCombatEvent(const CombatEvent& event) {
    if (m_eventDeduplicator.IsDuplicateAndRecord(event.eventId)) {
        return false;
    }

    if (m_onCombatEvent) {
        m_onCombatEvent(event);
    }
    return true;
}

bool ReplicationManager::ClientReceivePickupResult(const PickupResultEvent& event) {
    if (m_eventDeduplicator.IsDuplicateAndRecord(event.eventId)) {
        return false;
    }

    if (event.success) {
        m_registry.UnregisterEntity(event.targetLootEntity);
    }

    if (m_onPickupResult) {
        m_onPickupResult(event);
    }
    return true;
}

bool ReplicationManager::ClientReceiveEnvironmentalEvent(const EnvironmentalStateEvent& event) {
    if (m_eventDeduplicator.IsDuplicateAndRecord(event.eventId)) {
        return false;
    }

    if (m_onEnvironmentalEvent) {
        m_onEnvironmentalEvent(event);
    }
    return true;
}

bool ReplicationManager::ClientReceiveStormEvent(const StormStateEvent& event) {
    if (m_onStormEvent) {
        m_onStormEvent(event);
    }
    return true;
}

bool ReplicationManager::ClientGetInterpolatedEntity(NetworkEntityIdentity identity, EntitySnapshotRecord& outRecord) const {
    const float renderTime = m_clientTime - (m_config.snapshotInterpolationDelayMs / 1000.0f);
    WorldSnapshot sFrom, sTo;
    float alpha = 0.0f;

    if (!m_snapshotBuffer.GetInterpolationSnapshots(renderTime, sFrom, sTo, alpha)) {
        return false;
    }

    m_lastInterpolationAlpha = alpha;

    const auto* recFrom = sFrom.FindEntity(identity);
    const auto* recTo = sTo.FindEntity(identity);

    if (recFrom && recTo) {
        outRecord = m_interpolator.InterpolateRecord(*recFrom, *recTo, alpha);
        return true;
    } else if (recTo) {
        outRecord = *recTo;
        return true;
    } else if (recFrom) {
        outRecord = *recFrom;
        return true;
    }

    return false;
}

// ----------------------------------------------------------------------------
// Diagnostics
// ----------------------------------------------------------------------------

ReplicationDiagnostics ReplicationManager::GetDiagnostics() const {
    ReplicationDiagnostics diag;
    diag.totalReplicatedEntities = m_registry.GetActiveEntityCount();
    diag.playerCount = m_registry.GetEntityCountByType(NetworkEntityType::Player);
    diag.enemyCount = m_registry.GetEntityCountByType(NetworkEntityType::Enemy);
    diag.lootCount = m_registry.GetEntityCountByType(NetworkEntityType::Loot);
    diag.environmentalCount = m_registry.GetEntityCountByType(NetworkEntityType::EnvironmentalObject);

    diag.snapshotsSent = m_snapshotsSent;
    diag.snapshotsReceived = m_snapshotsReceived;
    diag.deltaSnapshotsSent = m_deltaSnapshotsSent;
    diag.deltaSnapshotsReceived = m_deltaSnapshotsReceived;
    diag.staleSnapshotsRejected = m_snapshotBuffer.GetRejectedStaleCount();
    diag.duplicateEventsRejected = m_eventDeduplicator.GetDuplicateCount();
    diag.combatEventsDispatched = m_combatEventsDispatched;
    diag.pickupTransactionsProcessed = m_pickupProcessed;
    diag.pickupTransactionsSuccessful = m_pickupSuccess;
    diag.pickupTransactionsRejected = m_pickupRejected;
    diag.currentSnapshotBufferSize = m_snapshotBuffer.GetSnapshotCount();
    diag.lastInterpolationAlpha = m_lastInterpolationAlpha;

    return diag;
}

} // namespace wildbound::network


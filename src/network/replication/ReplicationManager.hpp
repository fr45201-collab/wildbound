#pragma once

#include "NetworkEntityTypes.hpp"
#include "ReplicationRegistry.hpp"
#include "Snapshot.hpp"
#include "SnapshotBuffer.hpp"
#include "ReplicationInterpolator.hpp"
#include "ReplicationEvents.hpp"
#include "ReplicationConfiguration.hpp"
#include "../NetworkTypes.hpp"
#include "../NetworkMessage.hpp"
#include <memory>
#include <unordered_map>
#include <functional>
#include <string>

namespace wildbound::network {

struct ReplicationDiagnostics {
    size_t totalReplicatedEntities{ 0 };
    size_t playerCount{ 0 };
    size_t enemyCount{ 0 };
    size_t lootCount{ 0 };
    size_t environmentalCount{ 0 };
    uint64_t snapshotsSent{ 0 };
    uint64_t snapshotsReceived{ 0 };
    uint64_t deltaSnapshotsSent{ 0 };
    uint64_t deltaSnapshotsReceived{ 0 };
    uint64_t staleSnapshotsRejected{ 0 };
    uint64_t duplicateEventsRejected{ 0 };
    uint64_t combatEventsDispatched{ 0 };
    uint64_t pickupTransactionsProcessed{ 0 };
    uint64_t pickupTransactionsSuccessful{ 0 };
    uint64_t pickupTransactionsRejected{ 0 };
    size_t currentSnapshotBufferSize{ 0 };
    float lastInterpolationAlpha{ 0.0f };
    float bytesPerSecUpload{ 0.0f };
    float bytesPerSecDownload{ 0.0f };
};

using CombatEventCallback = std::function<void(const CombatEvent&)>;
using PickupResultCallback = std::function<void(const PickupResultEvent&)>;
using EnvironmentalEventCallback = std::function<void(const EnvironmentalStateEvent&)>;
using StormEventCallback = std::function<void(const StormStateEvent&)>;

/**
 * Master Replication Coordinator for Wildbound.
 * Implements authoritative server state simulation, input validation, combat hit resolution,
 * atomic loot transactions, environmental synchronization, and client-side interpolation.
 */
class ReplicationManager {
public:
    explicit ReplicationManager(NetworkRole role = NetworkRole::None);
    ~ReplicationManager() = default;

    /// Initialize replication coordinator with configuration
    bool Initialize(const std::string& configPath = "assets/config/network_replication.json");
    void Reset();

    /// Advance replication time and simulation tick
    void Update(float dt);

    /// Set engine network role (Server, Client, ListenServer, None)
    void SetRole(NetworkRole role) noexcept { m_role = role; }
    [[nodiscard]] NetworkRole GetRole() const noexcept { return m_role; }

    /// Access the central entity replication registry
    [[nodiscard]] ReplicationRegistry& GetRegistry() noexcept { return m_registry; }
    [[nodiscard]] const ReplicationRegistry& GetRegistry() const noexcept { return m_registry; }

    /// Access configuration
    [[nodiscard]] ReplicationConfiguration& GetConfig() noexcept { return m_config; }
    [[nodiscard]] const ReplicationConfiguration& GetConfig() const noexcept { return m_config; }

    /// Access client snapshot buffer
    [[nodiscard]] SnapshotBuffer& GetSnapshotBuffer() noexcept { return m_snapshotBuffer; }
    [[nodiscard]] const SnapshotBuffer& GetSnapshotBuffer() const noexcept { return m_snapshotBuffer; }

    /// Current server simulation tick
    [[nodiscard]] TickType GetServerTick() const noexcept { return m_serverTick; }
    void SetServerTick(TickType tick) noexcept { m_serverTick = tick; }

    // ------------------------------------------------------------------------
    // Server Authority: State Registration & Updates
    // ------------------------------------------------------------------------

    /// Server: Register a player entity
    NetworkEntityIdentity RegisterPlayer(uint32_t localId, float x, float y);

    /// Server: Register an enemy entity
    NetworkEntityIdentity RegisterEnemy(uint32_t localId, float x, float y, float health = 100.0f, uint8_t aiState = 0);

    /// Server: Register a loot/world item entity
    NetworkEntityIdentity RegisterLoot(uint32_t localId, uint32_t itemDefId, float x, float y, uint32_t quantity = 1);

    /// Server: Register an environmental object (door, mechanism, chest)
    NetworkEntityIdentity RegisterEnvironment(uint32_t localId, float x, float y, uint16_t stateId = 0);

    /// Server: Remove and invalidate an entity
    bool DestroyEntity(NetworkEntityIdentity identity, uint8_t reasonCode = 0);

    /// Server: Validate and execute client movement input commands
    bool ServerProcessInput(NetworkEntityIdentity playerId, const MsgPlayerInput& input, float dt);

    /// Server: Validate and execute client attack request
    bool ServerProcessAttack(NetworkEntityIdentity attackerId, const MsgAttackRequest& request);

    /// Server: Validate and execute atomic loot pickup transaction
    bool ServerProcessPickup(const PickupRequestEvent& request);

    /// Server: Update environmental entity state
    bool ServerUpdateEnvironment(NetworkEntityIdentity entityId, uint16_t stateId, bool active, bool destroyed);

    /// Server: Update storm state
    void ServerUpdateStorm(uint8_t phase, float cx, float cy, float radius, float targetRadius, float dps);

    /// Server: Build a world state snapshot (full or delta)
    WorldSnapshot BuildSnapshot(bool forceFull = false);

    // ------------------------------------------------------------------------
    // Client Authority: Reception & Interpolation
    // ------------------------------------------------------------------------

    /// Client: Push received snapshot into buffer
    bool ClientReceiveSnapshot(const WorldSnapshot& snapshot);

    /// Client: Push received combat event through deduplicator
    bool ClientReceiveCombatEvent(const CombatEvent& event);

    /// Client: Push received pickup result
    bool ClientReceivePickupResult(const PickupResultEvent& event);

    /// Client: Push received environmental event
    bool ClientReceiveEnvironmentalEvent(const EnvironmentalStateEvent& event);

    /// Client: Push received storm event
    bool ClientReceiveStormEvent(const StormStateEvent& event);

    /// Client: Sample interpolated state for a given entity at the current render delay
    [[nodiscard]] bool ClientGetInterpolatedEntity(NetworkEntityIdentity identity, EntitySnapshotRecord& outRecord) const;

    // ------------------------------------------------------------------------
    // Event Callbacks & Subscriptions
    // ------------------------------------------------------------------------

    void SetCombatEventCallback(CombatEventCallback cb) { m_onCombatEvent = std::move(cb); }
    void SetPickupResultCallback(PickupResultCallback cb) { m_onPickupResult = std::move(cb); }
    void SetEnvironmentalCallback(EnvironmentalEventCallback cb) { m_onEnvironmentalEvent = std::move(cb); }
    void SetStormCallback(StormEventCallback cb) { m_onStormEvent = std::move(cb); }

    // ------------------------------------------------------------------------
    // Diagnostics & Metrics
    // ------------------------------------------------------------------------

    [[nodiscard]] ReplicationDiagnostics GetDiagnostics() const;

private:
    NetworkRole m_role{ NetworkRole::None };
    ReplicationConfiguration m_config;
    ReplicationRegistry m_registry;
    SnapshotBuffer m_snapshotBuffer;
    ReplicationInterpolator m_interpolator;
    EventDeduplicator m_eventDeduplicator;

    // Simulation timing
    TickType m_serverTick{ 0 };
    float m_serverTime{ 0.0f };
    float m_clientTime{ 0.0f };
    float m_snapshotTimer{ 0.0f };
    uint64_t m_nextEventId{ 1000 };

    // Server-side authoritative state storage
    std::unordered_map<uint64_t, EntitySnapshotRecord> m_authoritativeStates;
    std::unordered_map<uint64_t, EntitySnapshotRecord> m_baselineStates;

    // Callbacks
    CombatEventCallback m_onCombatEvent;
    PickupResultCallback m_onPickupResult;
    EnvironmentalEventCallback m_onEnvironmentalEvent;
    StormEventCallback m_onStormEvent;

    // Diagnostic counters
    uint64_t m_snapshotsSent{ 0 };
    uint64_t m_snapshotsReceived{ 0 };
    uint64_t m_deltaSnapshotsSent{ 0 };
    uint64_t m_deltaSnapshotsReceived{ 0 };
    uint64_t m_combatEventsDispatched{ 0 };
    uint64_t m_pickupProcessed{ 0 };
    uint64_t m_pickupSuccess{ 0 };
    uint64_t m_pickupRejected{ 0 };
    mutable float m_lastInterpolationAlpha{ 0.0f };
};

} // namespace wildbound::network


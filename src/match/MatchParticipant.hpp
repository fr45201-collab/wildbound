#pragma once

#include <raylib.h>
#include <string>
#include <cstdint>

namespace wildbound::match {

/**
 * Type of participant registered in the match.
 */
enum class ParticipantType {
    LocalHuman,
    RemoteHuman,
    AI
};

/**
 * Live status of a participant throughout the match lifecycle.
 */
enum class ParticipantStatus {
    Registered,
    Alive,
    Eliminated,
    Disconnected
};

/**
 * Authoritative cause of participant elimination.
 */
enum class EliminationCause {
    Combat,
    CorruptionStorm,
    Environment,
    Disconnect,
    Unknown
};

[[nodiscard]] const char* ParticipantTypeToString(ParticipantType type) noexcept;
[[nodiscard]] const char* ParticipantStatusToString(ParticipantStatus status) noexcept;
[[nodiscard]] const char* EliminationCauseToString(EliminationCause cause) noexcept;

/**
 * Detailed record of a participant's elimination event.
 */
struct EliminationInfo {
    float timestamp = 0.0f;
    EliminationCause cause = EliminationCause::Unknown;
    uint32_t killerParticipantId = 0;
    std::string killerName;
};

/**
 * Authoritative competitor entity abstraction.
 */
struct MatchParticipant {
    uint32_t participantId = 0;
    uint32_t teamId = 0;
    ParticipantType type = ParticipantType::LocalHuman;
    std::string displayName;
    uint32_t entityId = 0;
    ParticipantStatus status = ParticipantStatus::Registered;
    Vector2 spawnPosition{ 0.0f, 0.0f };
    EliminationInfo eliminationInfo;
    uint32_t kills = 0;
    float damageDealt = 0.0f;
    float damageReceived = 0.0f;
    float combatDamageReceived = 0.0f;
    float stormDamageReceived = 0.0f;
    float survivalDuration = 0.0f;
    uint32_t placement = 0;
    bool isWinner = false;

    [[nodiscard]] bool IsAlive() const noexcept { return status == ParticipantStatus::Alive; }
    [[nodiscard]] bool IsEliminated() const noexcept { return status == ParticipantStatus::Eliminated; }
    [[nodiscard]] bool IsLocalHuman() const noexcept { return type == ParticipantType::LocalHuman; }
};

} // namespace wildbound::match


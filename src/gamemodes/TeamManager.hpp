#pragma once

#include "Team.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>

namespace wildbound::match {
class MatchParticipantRegistry;
}

namespace wildbound::gamemodes {

/**
 * Centralized manager for team lifecycle, membership validation, and friendly queries.
 */
class TeamManager {
public:
    TeamManager() = default;
    ~TeamManager() = default;

    void SetMaxTeamSize(uint32_t size) noexcept { m_maxTeamSize = size; }
    [[nodiscard]] uint32_t GetMaxTeamSize() const noexcept { return m_maxTeamSize; }

    // Lifecycle & Membership
    TeamId CreateTeam(const std::string& name = "");
    bool AddParticipantToTeam(TeamId teamId, uint32_t participantId, match::MatchParticipantRegistry* registry = nullptr);
    bool RemoveParticipantFromTeam(uint32_t participantId, match::MatchParticipantRegistry* registry = nullptr);
    void ClearTeams(match::MatchParticipantRegistry* registry = nullptr);

    // Queries
    [[nodiscard]] Team* FindTeam(TeamId teamId);
    [[nodiscard]] const Team* FindTeam(TeamId teamId) const;
    [[nodiscard]] TeamId GetParticipantTeamId(uint32_t participantId) const;
    [[nodiscard]] Team* GetParticipantTeam(uint32_t participantId);
    [[nodiscard]] const Team* GetParticipantTeam(uint32_t participantId) const;

    [[nodiscard]] std::vector<uint32_t> GetTeamMembers(TeamId teamId) const;
    [[nodiscard]] std::vector<uint32_t> GetAliveTeamMembers(TeamId teamId, const match::MatchParticipantRegistry& registry) const;
    [[nodiscard]] size_t GetAliveTeamCount(const match::MatchParticipantRegistry& registry) const;
    [[nodiscard]] std::vector<TeamId> GetAliveTeamIds(const match::MatchParticipantRegistry& registry) const;
    [[nodiscard]] size_t GetTotalTeamCount() const noexcept { return m_teams.size(); }
    [[nodiscard]] const std::unordered_map<TeamId, Team>& GetAllTeams() const noexcept { return m_teams; }

    // Relationship queries
    [[nodiscard]] bool IsTeammate(uint32_t participantA, uint32_t participantB) const;
    [[nodiscard]] bool IsEnemy(uint32_t participantA, uint32_t participantB) const;

    // Elimination synchronization
    bool UpdateTeamEliminationStatus(const match::MatchParticipantRegistry& registry, float currentTimestamp = 0.0f);

    void Reset();

private:
    uint32_t m_maxTeamSize = 4;
    TeamId m_nextTeamId = 1;
    std::unordered_map<TeamId, Team> m_teams;
    std::unordered_map<uint32_t, TeamId> m_participantToTeam;
};

} // namespace wildbound::gamemodes


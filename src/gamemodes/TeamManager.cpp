#include "TeamManager.hpp"
#include "match/MatchParticipantRegistry.hpp"
#include "core/Logger.hpp"

using wildbound::core::Logger;

namespace wildbound::gamemodes {

TeamId TeamManager::CreateTeam(const std::string& name) {
    TeamId id = m_nextTeamId++;
    Team team;
    team.id = id;
    team.name = name.empty() ? ("Team " + std::to_string(id)) : name;
    team.eliminated = false;
    team.placement = 0;
    team.eliminationTimestamp = 0.0f;

    m_teams[id] = team;
    return id;
}

bool TeamManager::AddParticipantToTeam(TeamId teamId, uint32_t participantId, match::MatchParticipantRegistry* registry) {
    if (participantId == 0) {
        Logger::Warn("TEAM_MGR", "Cannot add participant with invalid ID 0");
        return false;
    }

    auto teamIt = m_teams.find(teamId);
    if (teamIt == m_teams.end()) {
        Logger::Warn("TEAM_MGR", "Team ID " + std::to_string(teamId) + " does not exist.");
        return false;
    }

    Team& team = teamIt->second;

    // Check capacity
    if (team.memberParticipantIds.size() >= m_maxTeamSize) {
        Logger::Warn("TEAM_MGR", "Team " + std::to_string(teamId) + " is already at maximum capacity (" + std::to_string(m_maxTeamSize) + ").");
        return false;
    }

    // Check if participant is already in this team
    if (team.ContainsMember(participantId)) {
        return true;
    }

    // Check if participant already belongs to another team -> remove first
    auto existingTeamIt = m_participantToTeam.find(participantId);
    if (existingTeamIt != m_participantToTeam.end() && existingTeamIt->second != teamId) {
        RemoveParticipantFromTeam(participantId, registry);
    }

    team.memberParticipantIds.push_back(participantId);
    m_participantToTeam[participantId] = teamId;

    if (registry) {
        auto* participant = registry->FindParticipant(participantId);
        if (participant) {
            participant->teamId = teamId;
        }
    }

    return true;
}

bool TeamManager::RemoveParticipantFromTeam(uint32_t participantId, match::MatchParticipantRegistry* registry) {
    auto mapIt = m_participantToTeam.find(participantId);
    if (mapIt == m_participantToTeam.end()) {
        return false;
    }

    TeamId teamId = mapIt->second;
    m_participantToTeam.erase(mapIt);

    auto teamIt = m_teams.find(teamId);
    if (teamIt != m_teams.end()) {
        auto& members = teamIt->second.memberParticipantIds;
        members.erase(std::remove(members.begin(), members.end(), participantId), members.end());
    }

    if (registry) {
        auto* participant = registry->FindParticipant(participantId);
        if (participant) {
            participant->teamId = 0;
        }
    }

    return true;
}

void TeamManager::ClearTeams(match::MatchParticipantRegistry* registry) {
    if (registry) {
        for (const auto& [pid, _] : m_participantToTeam) {
            auto* participant = registry->FindParticipant(pid);
            if (participant) {
                participant->teamId = 0;
            }
        }
    }
    m_teams.clear();
    m_participantToTeam.clear();
    m_nextTeamId = 1;
}

Team* TeamManager::FindTeam(TeamId teamId) {
    auto it = m_teams.find(teamId);
    return (it != m_teams.end()) ? &it->second : nullptr;
}

const Team* TeamManager::FindTeam(TeamId teamId) const {
    auto it = m_teams.find(teamId);
    return (it != m_teams.end()) ? &it->second : nullptr;
}

TeamId TeamManager::GetParticipantTeamId(uint32_t participantId) const {
    auto it = m_participantToTeam.find(participantId);
    return (it != m_participantToTeam.end()) ? it->second : 0;
}

Team* TeamManager::GetParticipantTeam(uint32_t participantId) {
    TeamId id = GetParticipantTeamId(participantId);
    return (id != 0) ? FindTeam(id) : nullptr;
}

const Team* TeamManager::GetParticipantTeam(uint32_t participantId) const {
    TeamId id = GetParticipantTeamId(participantId);
    return (id != 0) ? FindTeam(id) : nullptr;
}

std::vector<uint32_t> TeamManager::GetTeamMembers(TeamId teamId) const {
    const auto* team = FindTeam(teamId);
    return team ? team->memberParticipantIds : std::vector<uint32_t>{};
}

std::vector<uint32_t> TeamManager::GetAliveTeamMembers(TeamId teamId, const match::MatchParticipantRegistry& registry) const {
    const auto* team = FindTeam(teamId);
    return team ? team->GetAliveMembers(registry) : std::vector<uint32_t>{};
}

size_t TeamManager::GetAliveTeamCount(const match::MatchParticipantRegistry& registry) const {
    size_t count = 0;
    for (const auto& [id, team] : m_teams) {
        if (!team.eliminated && team.GetAliveMemberCount(registry) > 0) {
            ++count;
        }
    }
    return count;
}

std::vector<TeamId> TeamManager::GetAliveTeamIds(const match::MatchParticipantRegistry& registry) const {
    std::vector<TeamId> alive;
    alive.reserve(m_teams.size());
    for (const auto& [id, team] : m_teams) {
        if (!team.eliminated && team.GetAliveMemberCount(registry) > 0) {
            alive.push_back(id);
        }
    }
    return alive;
}

bool TeamManager::IsTeammate(uint32_t participantA, uint32_t participantB) const {
    if (participantA == 0 || participantB == 0) return false;
    if (participantA == participantB) return true; // Self is on same team

    TeamId teamA = GetParticipantTeamId(participantA);
    TeamId teamB = GetParticipantTeamId(participantB);

    if (teamA == 0 || teamB == 0) return false;
    return teamA == teamB;
}

bool TeamManager::IsEnemy(uint32_t participantA, uint32_t participantB) const {
    if (participantA == 0 || participantB == 0) return false;
    if (participantA == participantB) return false;

    TeamId teamA = GetParticipantTeamId(participantA);
    TeamId teamB = GetParticipantTeamId(participantB);

    // If either has no team or they have different teams, they are enemies in BR
    return (teamA != teamB) || (teamA == 0);
}

bool TeamManager::UpdateTeamEliminationStatus(const match::MatchParticipantRegistry& registry, float currentTimestamp) {
    bool anyTeamEliminatedThisCheck = false;
    for (auto& [id, team] : m_teams) {
        if (!team.eliminated) {
            if (team.GetAliveMemberCount(registry) == 0 && !team.memberParticipantIds.empty()) {
                team.eliminated = true;
                team.eliminationTimestamp = currentTimestamp;
                anyTeamEliminatedThisCheck = true;
                Logger::Info("TEAM_MGR", "Team [" + team.name + "] (ID " + std::to_string(id) + ") ELIMINATED at " + std::to_string(currentTimestamp) + "s.");
            }
        }
    }
    return anyTeamEliminatedThisCheck;
}

void TeamManager::Reset() {
    m_teams.clear();
    m_participantToTeam.clear();
    m_nextTeamId = 1;
    m_maxTeamSize = 4;
}

} // namespace wildbound::gamemodes

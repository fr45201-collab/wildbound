#include "MatchAssignmentManager.hpp"

namespace wildbound::coordinator {

MatchAssignment* MatchAssignmentManager::CreateAssignment(const FormedMatch& formedMatch,
                                                          const ServerInstance& server,
                                                          const SessionManager& sessionMgr,
                                                          float currentTime,
                                                          float timeoutSeconds) {
    if (formedMatch.matchId.empty()) return nullptr;

    MatchAssignment assignment;
    assignment.matchId = formedMatch.matchId;
    assignment.assignedServerId = server.serverId;
    assignment.serverHost = server.hostAddress;
    assignment.serverPort = server.port;
    assignment.gameMode = formedMatch.gameMode;
    assignment.mapName = server.mapName;
    assignment.state = AssignmentState::PendingReservation;
    assignment.reservationStartTime = currentTime;
    assignment.timeoutSeconds = timeoutSeconds;

    uint32_t teamCounter = 1;
    for (const auto& party : formedMatch.parties) {
        for (SessionId sid : party.members) {
            const auto* s = sessionMgr.GetSession(sid);
            std::string name = s ? s->displayName : ("Player_" + std::to_string(sid));
            std::string token = "token_" + formedMatch.matchId + "_" + std::to_string(sid);
            std::string persistentId = s ? s->persistentUserId : "";

            assignment.participants.push_back(ParticipantReservation{
                .sessionId = sid,
                .displayName = name,
                .matchAuthToken = token,
                .teamId = teamCounter,
                .persistentUserId = persistentId
            });
            m_sessionToMatch[sid] = formedMatch.matchId;
        }
        ++teamCounter;
    }

    auto [it, inserted] = m_assignments.insert_or_assign(formedMatch.matchId, std::move(assignment));
    return &it->second;
}

MatchAssignment* MatchAssignmentManager::GetAssignment(const std::string& matchId) noexcept {
    auto it = m_assignments.find(matchId);
    return (it != m_assignments.end()) ? &it->second : nullptr;
}

const MatchAssignment* MatchAssignmentManager::GetAssignment(const std::string& matchId) const noexcept {
    auto it = m_assignments.find(matchId);
    return (it != m_assignments.end()) ? &it->second : nullptr;
}

MatchAssignment* MatchAssignmentManager::GetAssignmentBySession(SessionId sid) noexcept {
    auto it = m_sessionToMatch.find(sid);
    if (it == m_sessionToMatch.end()) return nullptr;
    return GetAssignment(it->second);
}

bool MatchAssignmentManager::ConfirmReservation(const std::string& matchId) {
    auto* a = GetAssignment(matchId);
    if (!a) return false;
    if (a->state != AssignmentState::PendingReservation) return false;
    a->state = AssignmentState::Confirmed;
    return true;
}

bool MatchAssignmentManager::RejectReservation(const std::string& matchId) {
    auto* a = GetAssignment(matchId);
    if (!a) return false;
    a->state = AssignmentState::Failed;

    // Clear session-to-match references
    for (const auto& p : a->participants) {
        m_sessionToMatch.erase(p.sessionId);
    }
    return true;
}

bool MatchAssignmentManager::CompleteMatch(const std::string& matchId) {
    auto* a = GetAssignment(matchId);
    if (!a) return false;
    a->state = AssignmentState::Completed;

    for (const auto& p : a->participants) {
        m_sessionToMatch.erase(p.sessionId);
    }
    return true;
}

void MatchAssignmentManager::CheckReservationTimeouts(float currentTime, std::vector<std::string>& outTimedOutMatches) {
    outTimedOutMatches.clear();
    for (auto& [id, a] : m_assignments) {
        if (a.IsTimedOut(currentTime)) {
            a.state = AssignmentState::Failed;
            outTimedOutMatches.push_back(id);
            for (const auto& p : a.participants) {
                m_sessionToMatch.erase(p.sessionId);
            }
        }
    }
}

void MatchAssignmentManager::Clear() {
    m_assignments.clear();
    m_sessionToMatch.clear();
}

} // namespace wildbound::coordinator


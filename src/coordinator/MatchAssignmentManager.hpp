#pragma once

#include "MatchAssignment.hpp"
#include "MatchmakingQueue.hpp"
#include "DedicatedServerRegistry.hpp"
#include "SessionManager.hpp"
#include <unordered_map>
#include <vector>
#include <string>

namespace wildbound::coordinator {

class MatchAssignmentManager {
public:
    MatchAssignmentManager() = default;
    ~MatchAssignmentManager() = default;

    MatchAssignment* CreateAssignment(const FormedMatch& formedMatch,
                                      const ServerInstance& server,
                                      const SessionManager& sessionMgr,
                                      float currentTime,
                                      float timeoutSeconds = 5.0f);

    [[nodiscard]] MatchAssignment* GetAssignment(const std::string& matchId) noexcept;
    [[nodiscard]] const MatchAssignment* GetAssignment(const std::string& matchId) const noexcept;
    [[nodiscard]] MatchAssignment* GetAssignmentBySession(SessionId sid) noexcept;

    bool ConfirmReservation(const std::string& matchId);
    bool RejectReservation(const std::string& matchId);
    bool CompleteMatch(const std::string& matchId);

    void CheckReservationTimeouts(float currentTime, std::vector<std::string>& outTimedOutMatches);

    [[nodiscard]] size_t GetActiveAssignmentCount() const noexcept { return m_assignments.size(); }
    [[nodiscard]] const std::unordered_map<std::string, MatchAssignment>& GetAllAssignments() const noexcept { return m_assignments; }

    void Clear();

private:
    std::unordered_map<std::string, MatchAssignment> m_assignments;
    std::unordered_map<SessionId, std::string> m_sessionToMatch;
};

} // namespace wildbound::coordinator


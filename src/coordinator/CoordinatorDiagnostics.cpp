#include "CoordinatorDiagnostics.hpp"
#include <sstream>
#include <iomanip>

namespace wildbound::coordinator {

void CoordinatorDiagnostics::Reset() {
    activeSessions = 0;
    activeParties = 0;
    activeLobbies = 0;
    pendingInvitations = 0;
    queuedParties = 0;
    registeredServers = 0;
    availableServers = 0;
    matchesFormed = 0;
    matchesCompleted = 0;
    reservationFailures = 0;
    uptimeSeconds = 0.0f;
    totalPacketsProcessed = 0;
}

void CoordinatorDiagnostics::Update(float dt) {
    uptimeSeconds += dt;
}

std::string CoordinatorDiagnostics::FormatSummary() const {
    std::ostringstream ss;
    ss << "=== COORDINATOR DIAGNOSTICS ===\n"
       << "  Uptime: " << std::fixed << std::setprecision(1) << uptimeSeconds << "s\n"
       << "  Sessions: " << activeSessions
       << " | Parties: " << activeParties
       << " | Lobbies: " << activeLobbies << "\n"
       << "  Pending Invites: " << pendingInvitations
       << " | Queued Parties: " << queuedParties << "\n"
       << "  Servers: " << registeredServers << " registered (" << availableServers << " available)\n"
       << "  Matches: " << matchesFormed << " formed, " << matchesCompleted << " completed, "
       << reservationFailures << " reservation failures\n"
       << "  Total Packets Processed: " << totalPacketsProcessed << "\n"
       << "================================";
    return ss.str();
}

} // namespace wildbound::coordinator


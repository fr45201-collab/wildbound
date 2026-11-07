#include "MatchSpawner.hpp"
#include "../world/WorldManager.hpp"
#include "../core/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace wildbound::match {

bool MatchSpawner::InitializeSpawnPoints(const world::WorldManager& worldMgr, const Rectangle& playableBounds) {
    m_spawnPoints.clear();

    const auto& regions = worldMgr.GetSpawnRegions();
    for (const auto& reg : regions) {
        for (const auto& pt : reg.spawnPoints) {
            // Verify within playableBounds
            if (pt.x >= playableBounds.x && pt.x <= (playableBounds.x + playableBounds.width) &&
                pt.y >= playableBounds.y && pt.y <= (playableBounds.y + playableBounds.height)) {
                MatchSpawnPoint sp;
                sp.position = pt;
                sp.regionId = reg.regionId;
                sp.isOccupied = false;
                sp.assignedParticipantId = 0;
                m_spawnPoints.push_back(sp);
            }
        }
    }

    return !m_spawnPoints.empty();
}

void MatchSpawner::AddCustomSpawnPoint(const Vector2& pos, const std::string& region) {
    MatchSpawnPoint sp;
    sp.position = pos;
    sp.regionId = region;
    sp.isOccupied = false;
    sp.assignedParticipantId = 0;
    m_spawnPoints.push_back(sp);
}

bool MatchSpawner::AssignSpawnPoints(std::vector<MatchParticipant>& participants,
                                     SpawnSelectionStrategy strategy,
                                     unsigned int randomSeed) {
    if (participants.empty()) {
        return true;
    }

    if (GetAvailableSpawnCount() < participants.size()) {
        return false; // Insufficient spawn points
    }

    std::mt19937 rng(randomSeed);

    if (strategy == SpawnSelectionStrategy::Random) {
        std::vector<size_t> availableIndices;
        for (size_t i = 0; i < m_spawnPoints.size(); ++i) {
            if (!m_spawnPoints[i].isOccupied) {
                availableIndices.push_back(i);
            }
        }
        std::shuffle(availableIndices.begin(), availableIndices.end(), rng);

        for (size_t i = 0; i < participants.size(); ++i) {
            size_t spawnIdx = availableIndices[i];
            m_spawnPoints[spawnIdx].isOccupied = true;
            m_spawnPoints[spawnIdx].assignedParticipantId = participants[i].participantId;
            participants[i].spawnPosition = m_spawnPoints[spawnIdx].position;
        }
    } else if (strategy == SpawnSelectionStrategy::MinimumDistance) {
        // Greedily pick spawn points that maximize distance between participants
        std::vector<size_t> chosenIndices;
        for (auto& p : participants) {
            int bestIdx = -1;
            float maxMinDist = -1.0f;

            for (size_t i = 0; i < m_spawnPoints.size(); ++i) {
                if (m_spawnPoints[i].isOccupied) continue;

                if (chosenIndices.empty()) {
                    bestIdx = static_cast<int>(i);
                    break;
                }

                float minDistToChosen = 1e9f;
                for (size_t c : chosenIndices) {
                    float dx = m_spawnPoints[i].position.x - m_spawnPoints[c].position.x;
                    float dy = m_spawnPoints[i].position.y - m_spawnPoints[c].position.y;
                    float d = std::sqrt(dx * dx + dy * dy);
                    if (d < minDistToChosen) minDistToChosen = d;
                }

                if (minDistToChosen > maxMinDist) {
                    maxMinDist = minDistToChosen;
                    bestIdx = static_cast<int>(i);
                }
            }

            if (bestIdx >= 0) {
                size_t sIdx = static_cast<size_t>(bestIdx);
                m_spawnPoints[sIdx].isOccupied = true;
                m_spawnPoints[sIdx].assignedParticipantId = p.participantId;
                p.spawnPosition = m_spawnPoints[sIdx].position;
                chosenIndices.push_back(sIdx);
            } else {
                return false;
            }
        }
    } else {
        // RegionalDistributed: round-robin or evenly distributed across regions
        std::vector<size_t> availableIndices;
        for (size_t i = 0; i < m_spawnPoints.size(); ++i) {
            if (!m_spawnPoints[i].isOccupied) {
                availableIndices.push_back(i);
            }
        }

        // Shuffle within deterministic seed
        std::shuffle(availableIndices.begin(), availableIndices.end(), rng);

        for (size_t i = 0; i < participants.size(); ++i) {
            size_t spawnIdx = availableIndices[i];
            m_spawnPoints[spawnIdx].isOccupied = true;
            m_spawnPoints[spawnIdx].assignedParticipantId = participants[i].participantId;
            participants[i].spawnPosition = m_spawnPoints[spawnIdx].position;
        }
    }

    return true;
}

bool MatchSpawner::AssignTeamSpawnPoints(std::vector<MatchParticipant>& participants, unsigned int randomSeed) {
    if (participants.empty()) return true;
    if (GetAvailableSpawnCount() < participants.size()) {
        core::Logger::Error("MATCH_SPAWN", "Insufficient available spawn points for team assignment.");
        return false;
    }

    // Group participants by teamId
    std::unordered_map<uint32_t, std::vector<size_t>> teams;
    std::vector<size_t> soloIndices;

    for (size_t i = 0; i < participants.size(); ++i) {
        if (participants[i].teamId != 0) {
            teams[participants[i].teamId].push_back(i);
        } else {
            soloIndices.push_back(i);
        }
    }

    std::mt19937 rng(randomSeed);

    for (auto& [teamId, memberIndices] : teams) {
        if (memberIndices.empty()) continue;

        std::vector<size_t> available;
        for (size_t i = 0; i < m_spawnPoints.size(); ++i) {
            if (!m_spawnPoints[i].isOccupied) available.push_back(i);
        }
        if (available.empty()) return false;

        std::shuffle(available.begin(), available.end(), rng);
        size_t leaderSpawnIdx = available[0];

        // Assign first member
        size_t firstMemberIdx = memberIndices[0];
        m_spawnPoints[leaderSpawnIdx].isOccupied = true;
        m_spawnPoints[leaderSpawnIdx].assignedParticipantId = participants[firstMemberIdx].participantId;
        participants[firstMemberIdx].spawnPosition = m_spawnPoints[leaderSpawnIdx].position;
        Vector2 leaderPos = m_spawnPoints[leaderSpawnIdx].position;

        // Assign subsequent members to nearest available spawns to leader
        for (size_t m = 1; m < memberIndices.size(); ++m) {
            size_t memberIdx = memberIndices[m];
            int closestSpawnIdx = -1;
            float closestDistSq = 1e18f;

            for (size_t s = 0; s < m_spawnPoints.size(); ++s) {
                if (m_spawnPoints[s].isOccupied) continue;
                float dx = m_spawnPoints[s].position.x - leaderPos.x;
                float dy = m_spawnPoints[s].position.y - leaderPos.y;
                float dsq = dx * dx + dy * dy;
                if (dsq < closestDistSq) {
                    closestDistSq = dsq;
                    closestSpawnIdx = static_cast<int>(s);
                }
            }

            if (closestSpawnIdx != -1) {
                m_spawnPoints[closestSpawnIdx].isOccupied = true;
                m_spawnPoints[closestSpawnIdx].assignedParticipantId = participants[memberIdx].participantId;
                participants[memberIdx].spawnPosition = m_spawnPoints[closestSpawnIdx].position;
            }
        }
    }

    // Assign any solo participants
    for (size_t soloIdx : soloIndices) {
        for (size_t s = 0; s < m_spawnPoints.size(); ++s) {
            if (!m_spawnPoints[s].isOccupied) {
                m_spawnPoints[s].isOccupied = true;
                m_spawnPoints[s].assignedParticipantId = participants[soloIdx].participantId;
                participants[soloIdx].spawnPosition = m_spawnPoints[s].position;
                break;
            }
        }
    }

    return true;
}

void MatchSpawner::ResetSpawns() {
    for (auto& sp : m_spawnPoints) {
        sp.isOccupied = false;
        sp.assignedParticipantId = 0;
    }
}

size_t MatchSpawner::GetAvailableSpawnCount() const {
    size_t count = 0;
    for (const auto& sp : m_spawnPoints) {
        if (!sp.isOccupied) {
            ++count;
        }
    }
    return count;
}

} // namespace wildbound::match


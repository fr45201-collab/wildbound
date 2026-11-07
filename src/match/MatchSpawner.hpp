#pragma once

#include "SpawnPoint.hpp"
#include "MatchParticipant.hpp"
#include <vector>
#include <random>

namespace wildbound::world {
    class WorldManager;
}

namespace wildbound::match {

enum class SpawnSelectionStrategy {
    RegionalDistributed,
    Random,
    MinimumDistance
};

/**
 * Spawning Coordinator for Battle Royale Competitors.
 * Loads and validates candidate spawn coordinates, applies distribution strategies,
 * verifies playable boundary constraints, and assigns participants.
 */
class MatchSpawner {
public:
    MatchSpawner() = default;
    ~MatchSpawner() = default;

    bool InitializeSpawnPoints(const world::WorldManager& worldMgr, const Rectangle& playableBounds);
    void AddCustomSpawnPoint(const Vector2& pos, const std::string& region = "custom");

    bool AssignSpawnPoints(std::vector<MatchParticipant>& participants,
                           SpawnSelectionStrategy strategy = SpawnSelectionStrategy::RegionalDistributed,
                           unsigned int randomSeed = 1337);

    bool AssignTeamSpawnPoints(std::vector<MatchParticipant>& participants,
                               unsigned int randomSeed = 1337);

    void ResetSpawns();

    [[nodiscard]] const std::vector<MatchSpawnPoint>& GetSpawnPoints() const noexcept { return m_spawnPoints; }
    [[nodiscard]] size_t GetAvailableSpawnCount() const;
    [[nodiscard]] size_t GetTotalSpawnCount() const noexcept { return m_spawnPoints.size(); }

private:
    std::vector<MatchSpawnPoint> m_spawnPoints;
};

} // namespace wildbound::match


#pragma once

#include <raylib.h>
#include <string>
#include <cstdint>

namespace wildbound::match {

/**
 * Validated Battle Royale Spawn Point.
 */
struct MatchSpawnPoint {
    Vector2 position{ 0.0f, 0.0f };
    std::string regionId;
    bool isOccupied = false;
    uint32_t assignedParticipantId = 0;
};

} // namespace wildbound::match


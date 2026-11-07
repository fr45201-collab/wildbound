#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace wildbound::entities {
class Player;
}

namespace wildbound::combat {
class CombatSystem;
}

namespace wildbound::world {
class EnemySpawner;
class World;
}

namespace wildbound::gamemodes {

using DummySpawnerCallback = std::function<uint32_t(Vector2)>;
using DummyClearCallback = std::function<void()>;

/**
 * Controller and state tracker for the dedicated Practice Arena.
 */
class PracticeMode {
public:
    PracticeMode();
    ~PracticeMode() = default;

    void Initialize(Vector2 defaultSpawnPos = { 100.0f, 100.0f });

    [[nodiscard]] bool IsActive() const noexcept { return m_isActive; }
    void SetActive(bool active) noexcept { m_isActive = active; }

    [[nodiscard]] bool IsGodMode() const noexcept { return m_godMode; }
    void SetGodMode(bool godMode) noexcept { m_godMode = godMode; }

    [[nodiscard]] bool IsDummySpawningEnabled() const noexcept { return m_dummySpawning; }
    void SetDummySpawningEnabled(bool enabled) noexcept { m_dummySpawning = enabled; }

    [[nodiscard]] Vector2 GetSpawnPosition() const noexcept { return m_spawnPosition; }
    void SetSpawnPosition(Vector2 pos) noexcept { m_spawnPosition = pos; }

    // Dummy callback configuration
    void SetDummyCallbacks(DummySpawnerCallback spawner, DummyClearCallback clearer) {
        m_dummySpawner = std::move(spawner);
        m_dummyClearer = std::move(clearer);
    }

    // Practice Interactive Actions
    void RestorePlayerHealth(entities::Player* player);
    void ResetPlayerPosition(entities::Player* player);
    void SpawnCombatDummy(Vector2 pos, world::EnemySpawner* spawner = nullptr, combat::CombatSystem* combatSys = nullptr);
    void ClearCombatDummies(world::EnemySpawner* spawner = nullptr, combat::CombatSystem* combatSys = nullptr);

    [[nodiscard]] size_t GetActiveDummyCount() const noexcept { return m_spawnedDummyIds.size(); }

    void Reset();

private:
    bool m_isActive = false;
    bool m_godMode = false;
    bool m_dummySpawning = true;
    Vector2 m_spawnPosition{ 100.0f, 100.0f };
    std::vector<uint32_t> m_spawnedDummyIds;

    DummySpawnerCallback m_dummySpawner;
    DummyClearCallback m_dummyClearer;
};

} // namespace wildbound::gamemodes


# Wildbound Combatant Architecture

The Combatant Architecture establishes the shared contract between all interactive combat entities in the game engine. By implementing the abstract `Combatant` interface, player characters, monsters, environmental hazards, and future summon/pet entities all share identical combat rules, hit detection, and damage models.

---

## 1. The `Combatant` Interface

Located at [`src/combat/Combatant.hpp`](file:///c:/dev/Wildbound/src/combat/Combatant.hpp), `Combatant` defines the fundamental pure virtual methods required for participating in combat:

```cpp
namespace wildbound::combat {

class Combatant {
public:
    virtual ~Combatant() = default;

    virtual CombatantId GetCombatantId() const = 0;
    virtual CombatantType GetCombatantType() const = 0;
    virtual Faction GetFaction() const = 0;

    virtual Vector2 GetCombatPosition() const = 0;
    virtual Rectangle GetHurtbox() const = 0;
    virtual graphics::animation::Direction GetCombatFacing() const = 0;

    virtual bool IsAlive() const = 0;
    virtual HealthComponent& GetHealth() = 0;
    virtual const HealthComponent& GetHealth() const = 0;

    virtual AttackSystem& GetAttackSystem() = 0;
    virtual const AttackSystem& GetAttackSystem() const = 0;

    virtual void OnDamageReceived(const DamageEvent& evt) = 0;
    virtual void OnDeath(const DeathEvent& evt) = 0;
};

}
```

---

## 2. Participant Implementations

### Human Player (`Player`)
- Defined in [`src/entities/Player.hpp`](file:///c:/dev/Wildbound/src/entities/Player.hpp) / [`.cpp`](file:///c:/dev/Wildbound/src/entities/Player.cpp).
- Inherits from `wildbound::entities::Character` and implements `wildbound::combat::Combatant`.
- Returns `CombatantType::Player` and `Faction::Players`.
- Hurtbox maps directly to `GetWorldBounds()`.
- Dispatches primary attacks via `Player::TryAttack(combatSys, "slash")` triggered by `Spacebar` or `Left Mouse Button`.

### AI Monster (`Enemy`)
- Defined in [`src/entities/Enemy.hpp`](file:///c:/dev/Wildbound/src/entities/Enemy.hpp) / [`.cpp`](file:///c:/dev/Wildbound/src/entities/Enemy.cpp).
- Inherits from `wildbound::entities::Character` and implements `wildbound::combat::Combatant`.
- Returns `CombatantType::Enemy` and `Faction::Monsters`.
- Hurtbox maps directly to `GetWorldBounds()`.
- Executes attacks via Lua AI decision trees or C++ state machine fallbacks through `CombatSystem::ProcessAttack`.

---

## 3. Faction Classification Matrix

The faction system categorizes combat entities to govern targeting rules:

```cpp
enum class Faction {
    None,
    Players,     // Human players (PvP rules apply)
    Monsters,    // Hostile monsters (Goblins, Skeletons)
    Neutral,     // Non-combatants or neutral wildlife
    Environment  // Traps, hazards, toxic zones
};
```

### Targeting Matrix Rules (`CombatRules`)

| Attacker \ Target | `Players` | `Monsters` | `Neutral` | `Environment` |
|---|---|---|---|---|
| **`Players`** | Only if `allowPvP` or `friendlyFire` | Allowed (`allowPvE`) | Prohibited | Prohibited |
| **`Monsters`** | Allowed (`allowPvE`) | Only if `friendlyFire` | Prohibited | Prohibited |
| **`Environment`** | Allowed | Allowed | Allowed | Prohibited |

---

## 4. Lifecycle and Registration

All entities entering active play register with the centralized `CombatSystem`:
```cpp
// In Game::Initialize or EnemySpawner::SpawnEnemy
m_combatSystem->RegisterCombatant(entityPtr);
```

When an entity is destroyed or removed from the scene:
```cpp
// Deferred removal in EnemySpawner::Update
combatSys.UnregisterCombatant(entity->GetCombatantId());
```

This registration pattern prevents stale pointer dereferences during spatial range queries or multi-target attacks.


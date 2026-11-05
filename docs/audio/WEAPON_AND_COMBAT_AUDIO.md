# Weapon and Combat Audio Specification

## 1. Overview & Mechanics

The Weapon and Combat Audio Subsystem connects physical melee swings, ranged archery mechanics, defensive parries/blocks, damage impacts, and entity mortality to the `AudioSystem`.

The integration leverages:
1. `wildbound::combat::CombatSystem`
2. `wildbound::combat::CombatEventDispatcher`
3. `wildbound::weapons::WeaponController`

---

## 2. Melee Weapon Audio

### 2.1 Sword Swings
- `weapon.sword.swing`: Triggered when an attack is initiated in `CombatSystem::ProcessAttack`.
  - Uses dual procedural variations (`sfx_sword_swing_001` for light attacks, `_002` for heavy slashes).
  - Sound: Air turbulence whoosh with subtle blade tone ($200\text{ Hz} \to 600\text{ Hz}$ sweep).

### 2.2 Impacts and Defenses
- `weapon.sword.impact`: Triggered when an attack connects with physical surfaces:
  - `sfx_sword_impact_metal`: Metallic clash ($1.48\text{ kHz} + 2.12\text{ kHz}$ resonant ring).
  - `sfx_sword_impact_wood`: Dull timber thud and splinter sound.
  - `sfx_sword_impact_stone`: Crisp flint ricochet snap.
- `weapon.shield.block`: Triggered on successful block mitigation ($80\text{ Hz}$ deep resonant thud).
- `weapon.parry`: High-frequency steel clink ($1.85\text{ kHz} + 2.64\text{ kHz}$) signaling offensive deflection.

---

## 3. Archery & Projectile Audio

### 3.1 Bow Mechanics
- `weapon.bow.draw`: Synthesized wood creak and string friction ($0.45\text{s}$) when the player charges bow draw.
- `weapon.bow.release`: Fast twang ($190\text{ Hz} \times e^{-15t}$) upon releasing the arrow.
- `weapon.bow.equip`: Leather buckle rustle and wood flex when switching to bow.

### 3.2 Arrow Trajectory & Impacts
- `weapon.arrow.flight`: Soft whistling air rush as projectile traverses world space.
- `weapon.arrow.impact_flesh`: Deep damp squish thump upon hitting a combatant entity.
- `weapon.arrow.impact_wood`: Crisp wood crack and embedding thud.
- `weapon.arrow.impact_stone`: High-frequency flint ricochet and fracture snap.

---

## 4. Combat Damage & Mortality Dispatch

In `CombatSystem::Initialize()`, `AudioSystem` subscribes directly to the `CombatEventDispatcher`:

### 4.1 Damage Events
```cpp
m_dispatcher.SubscribeDamage([this](const DamageEvent& evt) {
    if (!m_audioSystem) return;
    auto* target = GetCombatant(evt.targetId);
    if (!target) return;

    if (target->GetCombatantType() == CombatantType::Player) {
        m_audioSystem->DispatchEvent("combat.player.hurt");
    } else {
        auto* enemy = dynamic_cast<entities::Enemy*>(target);
        if (enemy) {
            m_audioSystem->DispatchEventSpatial("creature." + enemy->GetConfig().id + ".hurt", evt.hitPosition);
        } else {
            m_audioSystem->DispatchEventSpatial("combat.enemy.hurt", evt.hitPosition);
        }
    }

    if (evt.isCritical) {
        m_audioSystem->DispatchEventSpatial("combat.critical_hit", evt.hitPosition);
    } else {
        m_audioSystem->DispatchEventSpatial("combat.impact", evt.hitPosition);
    }
});
```

### 4.2 Death Events
```cpp
m_dispatcher.SubscribeDeath([this](const DeathEvent& evt) {
    if (!m_audioSystem) return;
    auto* victim = GetCombatant(evt.victimId);
    if (!victim) return;

    if (victim->GetCombatantType() == CombatantType::Player) {
        m_audioSystem->DispatchEvent("combat.player.death");
    } else {
        auto* enemy = dynamic_cast<entities::Enemy*>(victim);
        if (enemy) {
            m_audioSystem->DispatchEventSpatial("creature." + enemy->GetConfig().id + ".death", evt.deathPosition);
        } else {
            m_audioSystem->DispatchEventSpatial("combat.enemy.death", evt.deathPosition);
        }
    }
});
```

### 4.3 Cooldown Throttling
- Player hurt audio has a strict $0.25\text{s}$ cooldown to prevent audio distortion when hit by rapid damage ticks or multi-projectile bursts.
- Impacts have a $0.10\text{s}$ cooldown.


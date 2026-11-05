# Wildbound — Lua AI Integration & Scripting Hooks

## 1. Architecture & Script Binding

Wildbound embeds Lua 5.4 with the `sol2` binding framework to provide designers with rapid scripting capabilities for custom enemy behaviors, quest triggers, boss mechanics, and tactical overrides.

The C++ `AIController` and `Enemy` classes expose state update hooks and event dispatchers to Lua scripts located in `scripts/gameplay/ai/`.

---

## 2. Lua Script Layout

```
scripts/gameplay/ai/
├── enemy_base.lua      # Base enemy state machine, sensory dispatcher, and default actions
├── investigate.lua     # Investigation routine when hearing suspicious noise
├── defend.lua          # Defensive circling and pack posturing behavior
└── return.lua          # Safe return-to-spawn leash mechanics
```

---

## 3. C++ to Lua Dispatch Pipeline

During each simulation tick, if an entity has a registered Lua controller, `Enemy::UpdateAI` passes control to `enemy_base.lua`:

```lua
-- scripts/gameplay/ai/enemy_base.lua
function OnAIUpdate(entity, dt)
    local state = entity:GetAIState()
    
    if state == "Idle" then
        HandleIdle(entity, dt)
    elseif state == "Patrol" then
        HandlePatrol(entity, dt)
    elseif state == "Investigate" then
        HandleInvestigate(entity, dt)
    elseif state == "Alert" then
        HandleAlert(entity, dt)
    elseif state == "Chase" then
        HandleChase(entity, dt)
    elseif state == "Attack" then
        HandleAttack(entity, dt)
    elseif state == "Defend" then
        HandleDefend(entity, dt)
    elseif state == "Retreat" then
        HandleRetreat(entity, dt)
    elseif state == "Search" then
        HandleSearch(entity, dt)
    elseif state == "Return" then
        HandleReturn(entity, dt)
    end
end
```

---

## 4. Exposed API & Sol2 Bindings

Lua scripts receive full access to entity query and command functions:

```lua
-- Sample Lua Boss Ability Script
function CastShockwaveIfPlayerNear(boss)
    local target = boss:GetTarget()
    if target ~= nil then
        local dist = boss:DistanceTo(target)
        if dist < 48.0 and boss:CanCast("Shockwave") then
            boss:CastAbility("Shockwave")
            boss:PlayAnimation("attack_slam")
            boss:PlaySound("boss_slam_01")
        end
    end
end
```

### Safety & Native C++ Fallback
If a Lua script fails, throws a syntax error, or encounters a runtime nil-dereference:
1. `sol::protected_function` catches the exception cleanly.
2. The error message is logged to the debug console (`[ERROR] [LUA_AI] ...`).
3. The entity automatically falls back to its deterministic native C++ `AIController` logic without crashing the engine or disrupting other active entities.


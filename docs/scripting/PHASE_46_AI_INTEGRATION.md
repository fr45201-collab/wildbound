# Wildbound — Phase 46: Enemy AI & Gameplay Systems Integration

## 1. Overview

Wildbound's `wildbound::entities::Enemy` class represents simulated monsters (Goblins, Skeletons, etc.) inhabiting the world in both client and dedicated server builds.

In Phase 46, `Enemy::UpdateAI(float dt)` was upgraded to bridge directly into Lua scripting while preserving C++ authoritative validation.

---

## 2. Dispatch Pipeline

```
+-----------------------------------------------------------+
|               Enemy::UpdateAI(float dt)                   |
+-----------------------------------------------------------+
                             |
             [Check LuaManager Availability]
                             |
         +-------------------+-------------------+
         | Yes                                   | No (Fallback)
         v                                       v
[Route to Specific Archetype Function]     [ExecuteFallbackAI]
   - "goblin"   -> goblin_update
   - "skeleton" -> skeleton_update
   - default    -> enemy_update
         |
         v
[Invoke sol::protected_function]
         |
    +----+---------------------------+
    | Success                        | Error / Budget Exceeded
    v                                v
[ExecuteAIDecision(res)]       [Increment s_fallbackAICount]
                               [ExecuteFallbackAI]
```

---

## 3. Archetype Routing Rules

Archetype-specific routing eliminates generic script monolithic bottlenecks:
- If `m_type == "goblin"`: Calls `lua["wildbound"]["ai"]["goblin_update"]`.
- If `m_type == "skeleton"`: Calls `lua["wildbound"]["ai"]["skeleton_update"]`.
- If a custom script is specified in `m_config.behaviorScript`: Calls the configured function or `wildbound.ai.enemy_update`.

If the specific function is not bound in Lua, the system automatically falls back to `wildbound.ai.enemy_update` before falling back to pure C++.

---

## 4. Multi-Enemy Independence & Thread Safety

Because all situational context is passed into Lua as an ephemeral local table (`ctx`) and returned as an ephemeral table (`res`), no persistent cross-enemy state pollution occurs in Lua global variables.

In automated test suites, 20 distinct simulated enemies (10 Goblins and 10 Skeletons) with differing HP percentages and distances were ticked consecutively through the exact same Lua state with zero variable leakage or state cross-contamination.


# Wildbound — Phase 46: sol2 C++/Lua Bindings Specification

## 1. Overview

Wildbound utilizes **sol2** to expose engine data structures, utility mathematics, logging, and AI state enumerations to the Lua scripting environment. sol2 provides zero-overhead C++ abstractions with compile-time type checking and automated argument checking.

All bindings are scoped under the canonical global namespace table:
```lua
wildbound
```

---

## 2. Namespace Hierarchy

| Qualified Name | Type | Description |
| :--- | :--- | :--- |
| `wildbound` | Table | Root engine table |
| `wildbound.version` | String | Engine version string (`"Wildbound 0.1.0-alpha (Phase 46 Lua-Integrated)"`) |
| `wildbound.math` | Table | Mathematical vector and scalar helpers |
| `wildbound.ai` | Table | Root AI behavior namespace |
| `wildbound.ai.states` | Table | State reflection dictionary for AI status matching |
| `wildbound.log` | Table | Sandboxed engine logging utilities |

---

## 3. Vector2 Userdata Binding

`Vector2` is bound as a full first-class userdata type in Lua, compatible with raylib's `Vector2` layout:

```cpp
m_luaState->new_usertype<Vector2>("Vector2",
    sol::constructors<Vector2(), Vector2(float, float)>(),
    "x", &Vector2::x,
    "y", &Vector2::y,
    "length", [](const Vector2& v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    },
    "normalize", [](const Vector2& v) {
        float len = std::sqrt(v.x * v.x + v.y * v.y);
        if (len > 0.0001f) {
            return Vector2{ v.x / len, v.y / len };
        }
        return Vector2{ 0.0f, 0.0f };
    },
    "distance", [](const Vector2& a, const Vector2& b) {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }
);
```

### Lua Usage Example
```lua
local pos = Vector2.new(100.0, 150.0)
local target = Vector2.new(120.0, 150.0)
local dist = Vector2.distance(pos, target) -- Returns 20.0
local dir = (target - pos):normalize()
```

---

## 4. `wildbound.math` Bindings

High-performance C++ mathematical utility functions exposed to Lua:

```cpp
auto mathTable = wbTable.create_named("math");

// Distance between coordinates
mathTable["distance"] = [](float x1, float y1, float x2, float y2) -> float {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
};

// Normalize 2D vector
mathTable["normalize"] = [](float x, float y) -> std::tuple<float, float> {
    float len = std::sqrt(x * x + y * y);
    if (len > 0.0001f) {
        return { x / len, y / len };
    }
    return { 0.0f, 0.0f };
};

// Clamp scalar value
mathTable["clamp"] = [](float val, float minVal, float maxVal) -> float {
    return std::max(minVal, std::min(maxVal, val));
};

// Linear interpolation
mathTable["lerp"] = [](float a, float b, float t) -> float {
    return a + t * (b - a);
};
```

---

## 5. `wildbound.ai.states` Reflection Table

To prevent magic strings and state mismatch bugs between C++ and Lua, `wildbound.ai.states` exposes the authoritative state constants:

```lua
wildbound.ai.states = {
    IDLE = "IDLE",
    PATROL = "PATROL",
    CHASE = "CHASE",
    ATTACK = "ATTACK",
    RETREAT = "RETREAT",
    INVESTIGATE = "INVESTIGATE",
    DEFEND = "DEFEND",
    RETURN = "RETURN"
}
```

Scripts reference `wildbound.ai.states.ATTACK` instead of hardcoding raw strings.

---

## 6. `wildbound.log` Logging Bindings

Enables Lua scripts to log through the unified C++ `wildbound::core::Logger` system:

```lua
wildbound.log.info("Goblin detected enemy player at distance " .. dist)
wildbound.log.warn("Pathfinding target obstructed")
wildbound.log.error("Invalid combat parameter")
```

C++ delegates these to `Logger::Info("LUA", msg)`, `Logger::Warn("LUA", msg)`, and `Logger::Error("LUA", msg)` with timestamping, module tag filtering, and thread safety.


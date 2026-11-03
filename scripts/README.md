# Scripts Directory

Lua 5.4 scripting architecture for Wildbound runtime gameplay and configuration.

## Organization

- `config/` - Engine, input mapping, and system configuration tables (e.g. `engine.lua`).
- `gameplay/` - Gameplay mechanics, player state machines, interaction scripts.
- `systems/` - Periodic simulation systems, quest triggers, day/night cycles, weather.
- `ui/` - Dynamic UI bindings, menu state logic, dialogue event handlers.

The scripting runtime is bound to C++20 through `sol2`. All scripts must adhere to strict sandboxing, safe nil handling, and proper logging.


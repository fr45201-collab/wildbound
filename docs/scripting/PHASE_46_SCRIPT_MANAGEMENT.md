# Wildbound — Phase 46: Script Management, Caching & Development Hot Reload

## 1. Overview

`wildbound::scripting::LuaManager` implements an in-memory script registry and caching system that avoids redundant disk I/O while supporting hot reloading during active development.

---

## 2. Script Registry & Metadata Tracking

Every loaded script is registered with a `ScriptMetadata` record:

```cpp
struct ScriptMetadata {
    std::string path;
    std::filesystem::file_time_type lastModifiedTime;
    uint32_t executionCount = 0;
    double lastExecutionDurationMs = 0.0;
    bool isLoaded = false;
};
```

### 2.1 Caching Behavior
When `LoadScript(path, forceReload)` is called:
- If `forceReload == false` and `m_scripts.find(path)` indicates the script is already marked loaded, `LuaManager` skips file reads and parsing, returning `true` immediately.
- If `forceReload == true`, the file is re-read from disk, the Lua state executes the chunk, and `lastModifiedTime` is updated.

---

## 3. Development Hot Reload Mechanism

Hot reloading allows gameplay designers to adjust enemy AI timings, attack IDs, or retreat conditions without restarting the game or dedicated server.

### 3.1 Individual Script Reload
```cpp
bool LuaManager::ReloadScript(const std::string& scriptPath) {
    Logger::Info("SCRIPTING", "Reloading script: " + scriptPath);
    return LoadScript(scriptPath, true);
}
```

### 3.2 Global Script Reload
```cpp
bool LuaManager::ReloadAllScripts() {
    Logger::Info("SCRIPTING", "Reloading all registered Lua scripts...");
    bool allSuccess = true;
    for (const auto& [path, meta] : m_scripts) {
        if (!LoadScript(path, true)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}
```

---

## 4. DevUI Integration

In the Phase 2 Developer UI (`src/ui/DevUI.cpp`), Tab 22 ("Lua & Scripting") provides interactive controls for:
1. **Reload All Scripts Button**: Triggers `LuaManager::ReloadAllScripts()` instantaneously.
2. **Individual Script Reload Buttons**: Beside each row in the loaded scripts table, clicking `[Reload]` refreshes that specific file.
3. **Execution Diagnostics**: Displays total execution count and last execution duration for every registered file.


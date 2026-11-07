#include "LuaManager.hpp"
#include "../core/Logger.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#if __has_include(<raylib.h>)
#include <raylib.h>
#else
#ifndef VECTOR2_DEFINED
#define VECTOR2_DEFINED
struct Vector2 {
    float x{0.0f};
    float y{0.0f};
};
#endif
#endif
#include <filesystem>
#include <vector>
#include <chrono>
#include <cmath>

namespace fs = std::filesystem;
using wildbound::core::Logger;

namespace wildbound::scripting {

static void LuaInstructionCountHook(lua_State* L, lua_Debug* /*ar*/) {
    luaL_error(L, "Script exceeded instruction execution budget (infinite loop protection)");
}

LuaManager::LuaManager() = default;

LuaManager::~LuaManager() {
    Shutdown();
}

bool LuaManager::Initialize() {
    if (m_initialized) {
        Logger::Warn("SCRIPTING", "LuaManager is already initialized.");
        return true;
    }

    try {
        m_luaState = std::make_unique<sol::state>();
        m_luaState->open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::math,
            sol::lib::string,
            sol::lib::table
        );

        // Configure script search paths for modular require()
        std::string currentPath = (*m_luaState)["package"]["path"];
        std::string extraPaths = ";scripts/?.lua;scripts/?/init.lua;"
                                 "scripts/gameplay/ai/?.lua;scripts/gameplay/ai/behaviors/?.lua;"
                                 "../scripts/?.lua;../scripts/gameplay/ai/?.lua;?.lua";
        (*m_luaState)["package"]["path"] = currentPath + extraPaths;

        RegisterCoreBindings();
        m_initialized = true;
        Logger::Info("SCRIPTING", "Lua 5.4 runtime initialized with sol2 safety checks.");
        return true;
    }
    catch (const std::exception& ex) {
        Logger::Error("SCRIPTING", std::string("Failed to initialize Lua state: ") + ex.what());
        m_luaState.reset();
        m_initialized = false;
        return false;
    }
}

void LuaManager::Shutdown() {
    if (m_initialized) {
        Logger::Info("SCRIPTING", "Shutting down Lua runtime.");
        if (m_luaState) {
            EndExecutionBudget();
        }
        m_scripts.clear();
        m_luaState.reset();
        m_initialized = false;
    }
}

void LuaManager::BeginExecutionBudget() {
    if (m_budgetEnabled && m_luaState && m_instructionBudget > 0) {
        lua_sethook(m_luaState->lua_state(), LuaInstructionCountHook, LUA_MASKCOUNT, m_instructionBudget);
    }
}

void LuaManager::EndExecutionBudget() {
    if (m_luaState) {
        lua_sethook(m_luaState->lua_state(), nullptr, 0, 0);
    }
}

uint64_t LuaManager::GetFileTimestamp(const std::string& filepath) const {
    try {
        if (fs::exists(filepath)) {
            auto ftime = fs::last_write_time(filepath);
            return static_cast<uint64_t>(ftime.time_since_epoch().count());
        }
    } catch (...) {}
    return 0;
}

void LuaManager::RegisterCoreBindings() {
    if (!m_luaState) return;

    auto wildboundTable = m_luaState->create_named_table("wildbound");
    wildboundTable["version"] = "0.2.0-phase46";

    // Logging Bindings
    wildboundTable["log_info"] = [](const std::string& msg) {
        Logger::Info("LUA", msg);
    };
    wildboundTable["log_warn"] = [](const std::string& msg) {
        Logger::Warn("LUA", msg);
    };
    wildboundTable["log_error"] = [](const std::string& msg) {
        Logger::Error("LUA", msg);
    };
    wildboundTable["log_debug"] = [](const std::string& msg) {
        Logger::Debug("LUA", msg);
    };

    // Math Bindings
    auto mathTable = wildboundTable.create_named("math");
    mathTable["distance"] = [](float x1, float y1, float x2, float y2) -> float {
        float dx = x2 - x1;
        float dy = y2 - y1;
        return std::sqrt(dx * dx + dy * dy);
    };
    mathTable["normalize"] = [](float x, float y) -> std::pair<float, float> {
        float lenSq = x * x + y * y;
        if (lenSq > 0.0001f) {
            float invLen = 1.0f / std::sqrt(lenSq);
            return { x * invLen, y * invLen };
        }
        return { 0.0f, 0.0f };
    };
    mathTable["clamp"] = [](float val, float minVal, float maxVal) -> float {
        if (val < minVal) return minVal;
        if (val > maxVal) return maxVal;
        return val;
    };
    mathTable["lerp"] = [](float a, float b, float t) -> float {
        return a + t * (b - a);
    };

    // Vector2 Userdata
    m_luaState->new_usertype<Vector2>("Vector2",
        sol::constructors<Vector2(), Vector2(float, float)>(),
        "x", &Vector2::x,
        "y", &Vector2::y,
        "length", [](const Vector2& v) { return std::sqrt(v.x * v.x + v.y * v.y); },
        "distance", [](const Vector2& a, const Vector2& b) {
            float dx = a.x - b.x;
            float dy = a.y - b.y;
            return std::sqrt(dx * dx + dy * dy);
        },
        "normalize", [](const Vector2& v) {
            float len = std::sqrt(v.x * v.x + v.y * v.y);
            if (len > 0.0001f) return Vector2{ v.x / len, v.y / len };
            return Vector2{ 0.0f, 0.0f };
        }
    );

    // AI Namespace
    auto aiTable = wildboundTable.create_named("ai");
    aiTable["states"] = m_luaState->create_table_with(
        "IDLE", "IDLE",
        "PATROL", "PATROL",
        "DETECT_TARGET", "DETECT_TARGET",
        "CHASE", "CHASE",
        "ATTACK", "ATTACK",
        "RETREAT", "RETREAT",
        "DEAD", "DEAD",
        "INVESTIGATE", "INVESTIGATE",
        "ALERT", "ALERT",
        "DEFEND", "DEFEND",
        "SEARCH", "SEARCH",
        "RETURN", "RETURN"
    );
}

bool LuaManager::ExecuteString(const std::string& code) {
    if (!m_initialized || !m_luaState) {
        Logger::Error("SCRIPTING", "Cannot execute Lua string: runtime is not initialized.");
        return false;
    }

    auto start = std::chrono::high_resolution_clock::now();
    BeginExecutionBudget();

    auto result = m_luaState->safe_script(code, sol::script_pass_on_error);

    EndExecutionBudget();
    auto end = std::chrono::high_resolution_clock::now();
    double durationUs = std::chrono::duration<double, std::micro>(end - start).count();

    m_totalExecutions++;
    m_lastExecutionTimeUs = durationUs;

    if (!result.valid()) {
        sol::error err = result;
        std::string errStr = err.what();
        m_totalErrors++;
        if (errStr.find("instruction execution budget") != std::string::npos) {
            m_budgetViolationCount++;
            Logger::Error("SCRIPTING", "Execution budget violation: " + errStr);
        } else {
            Logger::Error("SCRIPTING", std::string("Lua script execution error: ") + errStr);
        }
        return false;
    }
    return true;
}

bool LuaManager::LoadScriptFile(const std::string& filepath) {
    return LoadScript(filepath, false);
}

bool LuaManager::LoadScript(const std::string& filepath, bool forceReload) {
    if (!m_initialized || !m_luaState) {
        Logger::Error("SCRIPTING", "Cannot load Lua file: runtime is not initialized.");
        return false;
    }

    // Resolve candidates
    std::string resolvedPath = filepath;
    if (!fs::exists(resolvedPath)) {
        std::vector<std::string> candidates = {
            "scripts/" + filepath,
            "../" + filepath,
            "../../" + filepath
        };
        for (const auto& c : candidates) {
            if (fs::exists(c)) {
                resolvedPath = c;
                break;
            }
        }
    }

    if (!fs::exists(resolvedPath)) {
        Logger::Error("SCRIPTING", "Script file not found: " + filepath);
        m_scripts[filepath].lastError = "File not found";
        m_scripts[filepath].isLoaded = false;
        m_totalErrors++;
        return false;
    }

    uint64_t currentTimestamp = GetFileTimestamp(resolvedPath);
    auto it = m_scripts.find(filepath);
    if (!forceReload && it != m_scripts.end() && it->second.isLoaded && it->second.lastModifiedTime == currentTimestamp) {
        // Already loaded and up to date
        return true;
    }

    auto start = std::chrono::high_resolution_clock::now();
    BeginExecutionBudget();

    auto result = m_luaState->safe_script_file(resolvedPath, sol::script_pass_on_error);

    EndExecutionBudget();
    auto end = std::chrono::high_resolution_clock::now();
    double durationUs = std::chrono::duration<double, std::micro>(end - start).count();

    ScriptMetadata& meta = m_scripts[filepath];
    meta.filepath = resolvedPath;
    meta.lastModifiedTime = currentTimestamp;
    meta.lastExecutionTimeUs = durationUs;
    meta.executionCount++;
    m_totalExecutions++;
    m_lastExecutionTimeUs = durationUs;

    if (!result.valid()) {
        sol::error err = result;
        std::string errStr = err.what();
        meta.errorCount++;
        meta.lastError = errStr;
        meta.isLoaded = false;
        m_totalErrors++;

        if (errStr.find("instruction execution budget") != std::string::npos) {
            m_budgetViolationCount++;
            Logger::Error("SCRIPTING", "Execution budget violation in '" + resolvedPath + "': " + errStr);
        } else {
            Logger::Error("SCRIPTING", "Failed to load script '" + resolvedPath + "': " + errStr);
        }
        return false;
    }

    meta.isLoaded = true;
    meta.lastError.clear();
    Logger::Info("SCRIPTING", "Script loaded: " + resolvedPath);
    return true;
}

bool LuaManager::ReloadScript(const std::string& filepath) {
    return LoadScript(filepath, true);
}

bool LuaManager::ReloadAllScripts() {
    Logger::Info("SCRIPTING", "Reloading all registered Lua scripts...");
    bool allSuccess = true;
    std::vector<std::string> paths;
    paths.reserve(m_scripts.size());
    for (const auto& [path, _] : m_scripts) {
        paths.push_back(path);
    }
    for (const auto& path : paths) {
        if (!LoadScript(path, true)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool LuaManager::IsScriptLoaded(const std::string& filepath) const {
    auto it = m_scripts.find(filepath);
    return it != m_scripts.end() && it->second.isLoaded;
}

void LuaManager::RecordExecution(const std::string& scriptPath, double durationUs, bool success, const std::string& error) {
    m_totalExecutions++;
    m_lastExecutionTimeUs = durationUs;

    auto it = m_scripts.find(scriptPath);
    if (it != m_scripts.end()) {
        it->second.executionCount++;
        it->second.lastExecutionTimeUs = durationUs;
        if (!success) {
            it->second.errorCount++;
            it->second.lastError = error;
        }
    }

    if (!success) {
        m_totalErrors++;
        if (error.find("instruction execution budget") != std::string::npos) {
            m_budgetViolationCount++;
        }
    }
}

size_t LuaManager::GetMemoryUsageKb() const {
    if (!m_luaState) return 0;
    int kb = lua_gc(m_luaState->lua_state(), LUA_GCCOUNT, 0);
    return kb >= 0 ? static_cast<size_t>(kb) : 0;
}

std::string LuaManager::ResolveConfigPath(const std::string& preferredPath) {
    if (!preferredPath.empty() && fs::exists(preferredPath)) {
        return preferredPath;
    }

    const std::vector<std::string> candidates = {
        preferredPath,
        "scripts/config/engine.lua",
        "config/engine.lua",
        "../scripts/config/engine.lua",
        "../config/engine.lua",
        "../../scripts/config/engine.lua",
        "../../config/engine.lua",
        "../../../scripts/config/engine.lua",
        "../../../config/engine.lua"
    };

    for (const auto& path : candidates) {
        if (!path.empty() && fs::exists(path)) {
            return path;
        }
    }
    return "";
}

bool LuaManager::LoadEngineConfig(const std::string& preferredPath, core::EngineConfig& outConfig) {
    if (!m_initialized || !m_luaState) {
        Logger::Error("SCRIPTING", "Cannot load engine config: Lua runtime not initialized.");
        return false;
    }

    std::string resolvedPath = ResolveConfigPath(preferredPath);
    if (resolvedPath.empty()) {
        Logger::Error("SCRIPTING", "Could not locate engine configuration file (searched scripts/config/ and config/).");
        return false;
    }

    Logger::Info("SCRIPTING", "Loading engine configuration from: " + resolvedPath);

    BeginExecutionBudget();
    auto result = m_luaState->safe_script_file(resolvedPath, sol::script_pass_on_error);
    EndExecutionBudget();

    if (!result.valid()) {
        sol::error err = result;
        Logger::Error("SCRIPTING", "Failed executing engine config script: " + std::string(err.what()));
        return false;
    }

    sol::object obj = result;
    if (!obj.is<sol::table>()) {
        Logger::Error("SCRIPTING", "engine.lua must return a configuration table.");
        return false;
    }

    sol::table root = obj.as<sol::table>();

    // Parse 'game' section
    sol::optional<sol::table> gameSection = root["game"];
    if (gameSection) {
        outConfig.gameTitle = gameSection->get_or<std::string>("title", outConfig.gameTitle);
        outConfig.gameVersion = gameSection->get_or<std::string>("version", outConfig.gameVersion);
        outConfig.author = gameSection->get_or<std::string>("author", outConfig.author);
    }

    // Parse 'display' section
    sol::optional<sol::table> displaySection = root["display"];
    if (displaySection) {
        outConfig.windowWidth = displaySection->get_or<int>("window_width", outConfig.windowWidth);
        outConfig.windowHeight = displaySection->get_or<int>("window_height", outConfig.windowHeight);
        outConfig.internalWidth = displaySection->get_or<int>("internal_width", outConfig.internalWidth);
        outConfig.internalHeight = displaySection->get_or<int>("internal_height", outConfig.internalHeight);
        outConfig.targetFps = displaySection->get_or<int>("target_fps", outConfig.targetFps);
        outConfig.vsync = displaySection->get_or<bool>("vsync", outConfig.vsync);
        outConfig.resizable = displaySection->get_or<bool>("resizable", outConfig.resizable);
        outConfig.fullscreen = displaySection->get_or<bool>("fullscreen", outConfig.fullscreen);
    }

    // Parse 'debug' section
    sol::optional<sol::table> debugSection = root["debug"];
    if (debugSection) {
        outConfig.debugMode = debugSection->get_or<bool>("enabled", outConfig.debugMode);
        outConfig.showMetricsOverlay = debugSection->get_or<bool>("show_metrics_overlay", outConfig.showMetricsOverlay);
        outConfig.logLevel = debugSection->get_or<std::string>("log_level", outConfig.logLevel);
    }

    if (!outConfig.Validate()) {
        Logger::Warn("CONFIG", "Engine configuration had validation warnings, check parameters.");
    }

    Logger::Info("LUA", "Configuration loaded successfully: " + outConfig.gameTitle +
                 " (" + std::to_string(outConfig.windowWidth) + "x" + std::to_string(outConfig.windowHeight) +
                 ", canvas " + std::to_string(outConfig.internalWidth) + "x" + std::to_string(outConfig.internalHeight) +
                 " @" + std::to_string(outConfig.targetFps) + "fps)");

    return true;
}

sol::state& LuaManager::GetState() {
    return *m_luaState;
}

lua_State* LuaManager::GetLuaState() {
    return m_luaState ? m_luaState->lua_state() : nullptr;
}

} // namespace wildbound::scripting

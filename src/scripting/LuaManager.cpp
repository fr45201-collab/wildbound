#include "LuaManager.hpp"
#include "../core/Logger.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;
using wildbound::core::Logger;

namespace wildbound::scripting {

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

        RegisterCoreBindings();
        m_initialized = true;
        Logger::Info("SCRIPTING", "Lua runtime initialized.");
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
        m_luaState.reset();
        m_initialized = false;
    }
}

void LuaManager::RegisterCoreBindings() {
    if (!m_luaState) return;

    auto wildboundTable = m_luaState->create_named_table("wildbound");
    wildboundTable["version"] = "0.1.0-phase0";

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
}

bool LuaManager::ExecuteString(const std::string& code) {
    if (!m_initialized || !m_luaState) {
        Logger::Error("SCRIPTING", "Cannot execute Lua string: runtime is not initialized.");
        return false;
    }

    auto result = m_luaState->safe_script(code, sol::script_pass_on_error);
    if (!result.valid()) {
        sol::error err = result;
        Logger::Error("SCRIPTING", std::string("Lua script execution error: ") + err.what());
        return false;
    }
    return true;
}

bool LuaManager::LoadScriptFile(const std::string& filepath) {
    if (!m_initialized || !m_luaState) {
        Logger::Error("SCRIPTING", "Cannot load Lua file: runtime is not initialized.");
        return false;
    }

    if (!fs::exists(filepath)) {
        Logger::Error("SCRIPTING", "Script file not found: " + filepath);
        return false;
    }

    auto result = m_luaState->safe_script_file(filepath, sol::script_pass_on_error);
    if (!result.valid()) {
        sol::error err = result;
        Logger::Error("SCRIPTING", "Failed to load script '" + filepath + "': " + err.what());
        return false;
    }
    return true;
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

    auto result = m_luaState->safe_script_file(resolvedPath, sol::script_pass_on_error);
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

} // namespace wildbound::scripting


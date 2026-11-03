#pragma once

#include "../core/EngineConfig.hpp"
#include <memory>
#include <string>

// Forward declaration of sol::state
namespace sol {
    class state;
}

namespace wildbound::scripting {

class LuaManager {
public:
    LuaManager();
    ~LuaManager();

    // Disable copying
    LuaManager(const LuaManager&) = delete;
    LuaManager& operator=(const LuaManager&) = delete;

    bool Initialize();
    void Shutdown();

    bool ExecuteString(const std::string& code);
    bool LoadScriptFile(const std::string& filepath);

    bool LoadEngineConfig(const std::string& preferredPath, core::EngineConfig& outConfig);

    sol::state& GetState();
    bool IsInitialized() const { return m_initialized; }

private:
    void RegisterCoreBindings();
    std::string ResolveConfigPath(const std::string& preferredPath);

    std::unique_ptr<sol::state> m_luaState;
    bool m_initialized = false;
};

} // namespace wildbound::scripting


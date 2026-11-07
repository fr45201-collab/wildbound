#pragma once

#include "../core/EngineConfig.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <cstdint>

// Forward declaration of sol::state and lua_State
namespace sol {
    class state;
}
struct lua_State;

namespace wildbound::scripting {

struct ScriptMetadata {
    std::string filepath;
    bool isLoaded{false};
    uint64_t lastModifiedTime{0};
    uint64_t executionCount{0};
    uint64_t errorCount{0};
    double lastExecutionTimeUs{0.0};
    std::string lastError;
};

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

    // Caching and hot reload
    bool LoadScript(const std::string& filepath, bool forceReload = false);
    bool ReloadScript(const std::string& filepath);
    bool ReloadAllScripts();
    bool IsScriptLoaded(const std::string& filepath) const;

    bool LoadEngineConfig(const std::string& preferredPath, core::EngineConfig& outConfig);

    sol::state& GetState();
    lua_State* GetLuaState();
    bool IsInitialized() const { return m_initialized; }

    // Instruction budget / Infinite loop protection
    void SetInstructionBudget(int budgetInstructions) noexcept { m_instructionBudget = budgetInstructions; }
    int GetInstructionBudget() const noexcept { return m_instructionBudget; }
    void EnableInstructionBudget(bool enable) noexcept { m_budgetEnabled = enable; }
    bool IsInstructionBudgetEnabled() const noexcept { return m_budgetEnabled; }
    uint64_t GetBudgetViolationCount() const noexcept { return m_budgetViolationCount; }

    // Scoped execution hook helpers
    void BeginExecutionBudget();
    void EndExecutionBudget();

    // Diagnostics & Performance Profiling
    size_t GetMemoryUsageKb() const;
    uint64_t GetTotalExecutionCount() const noexcept { return m_totalExecutions; }
    uint64_t GetTotalErrorCount() const noexcept { return m_totalErrors; }
    double GetLastExecutionTimeUs() const noexcept { return m_lastExecutionTimeUs; }
    const std::unordered_map<std::string, ScriptMetadata>& GetLoadedScripts() const noexcept { return m_scripts; }

    void RecordExecution(const std::string& scriptPath, double durationUs, bool success, const std::string& error = "");

private:
    void RegisterCoreBindings();
    std::string ResolveConfigPath(const std::string& preferredPath);
    uint64_t GetFileTimestamp(const std::string& filepath) const;

    std::unique_ptr<sol::state> m_luaState;
    bool m_initialized = false;

    // Script cache and metadata
    std::unordered_map<std::string, ScriptMetadata> m_scripts;

    // Instruction Budget
    int m_instructionBudget{100000}; // Default 100k instructions per execution
    bool m_budgetEnabled{true};
    uint64_t m_budgetViolationCount{0};

    // Diagnostics
    uint64_t m_totalExecutions{0};
    uint64_t m_totalErrors{0};
    double m_lastExecutionTimeUs{0.0};
};

} // namespace wildbound::scripting

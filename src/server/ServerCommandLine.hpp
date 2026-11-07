#pragma once

#include "ServerConfig.hpp"
#include <string>
#include <vector>

namespace wildbound::server {

class ServerCommandLine {
public:
    ServerCommandLine() = default;
    ~ServerCommandLine() = default;

    /// Parse CLI arguments and override config fields
    bool Parse(int argc, char* argv[], ServerConfig& inOutConfig);

    [[nodiscard]] bool ShouldShowHelp() const noexcept { return m_showHelp; }
    [[nodiscard]] bool ShouldShowVersion() const noexcept { return m_showVersion; }
    [[nodiscard]] bool IsTestRunMode() const noexcept { return m_testRunMode; }
    [[nodiscard]] const std::string& GetConfigFilePath() const noexcept { return m_configFilePath; }

    /// Print formatted help text to stdout
    static void PrintHelp();

    /// Print version information to stdout
    static void PrintVersion();

private:
    bool m_showHelp{ false };
    bool m_showVersion{ false };
    bool m_testRunMode{ false };
    std::string m_configFilePath{ "assets/config/server.json" };
};

} // namespace wildbound::server


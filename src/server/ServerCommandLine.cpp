#include "ServerCommandLine.hpp"
#include <iostream>
#include <string_view>

namespace wildbound::server {

bool ServerCommandLine::Parse(int argc, char* argv[], ServerConfig& inOutConfig) {
    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);

        if (arg == "--help" || arg == "-h") {
            m_showHelp = true;
            return true;
        } else if (arg == "--version" || arg == "-v") {
            m_showVersion = true;
            return true;
        } else if (arg == "--test-run" || arg == "--headless-verify") {
            m_testRunMode = true;
        } else if (arg == "--config" && i + 1 < argc) {
            m_configFilePath = argv[++i];
        } else if (arg == "--host" && i + 1 < argc) {
            inOutConfig.bindAddress = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            inOutConfig.port = static_cast<uint16_t>(std::stoi(argv[++i]));
        } else if (arg == "--max-clients" && i + 1 < argc) {
            inOutConfig.maxClients = static_cast<uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--tick-rate" && i + 1 < argc) {
            inOutConfig.tickRate = static_cast<uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--map" && i + 1 < argc) {
            inOutConfig.mapName = argv[++i];
        } else if (arg == "--mode" && i + 1 < argc) {
            inOutConfig.gameMode = argv[++i];
        } else if (arg == "--log-level" && i + 1 < argc) {
            inOutConfig.logLevel = argv[++i];
        } else if (arg == "--coordinator" && i + 1 < argc) {
            std::string coordStr = argv[++i];
            inOutConfig.enableCoordinator = true;
            auto colonPos = coordStr.find(':');
            if (colonPos != std::string::npos) {
                inOutConfig.coordinatorHost = coordStr.substr(0, colonPos);
                inOutConfig.coordinatorPort = static_cast<uint16_t>(std::stoi(coordStr.substr(colonPos + 1)));
            } else {
                inOutConfig.coordinatorHost = coordStr;
            }
        }
    }

    inOutConfig.Validate();
    return true;
}

void ServerCommandLine::PrintHelp() {
    std::cout << "========================================================\n"
              << "WILDBOUND DEDICATED SERVER\n"
              << "========================================================\n"
              << "Usage: WildboundServer.exe [options]\n\n"
              << "Options:\n"
              << "  --config <path>       Path to JSON configuration file (default: assets/config/server.json)\n"
              << "  --host <address>      IP address to bind server host (default: 0.0.0.0)\n"
              << "  --port <port>         UDP port to listen on (default: 7777)\n"
              << "  --max-clients <count> Maximum simultaneous connected clients (1-64, default: 32)\n"
              << "  --tick-rate <hz>      Simulation tick rate in Hz (10-120, default: 30)\n"
              << "  --map <path/name>     World map path or identifier to load\n"
              << "  --mode <name>         Game mode to host (e.g., 'Solo Battle Royale')\n"
              << "  --log-level <level>   Log verbosity: trace, debug, info, warn, error\n"
              << "  --test-run            Initialize server, verify headless systems, and exit cleanly\n"
              << "  --help, -h            Show this help manual\n"
              << "  --version, -v         Display server version and build target\n"
              << "========================================================\n";
}

void ServerCommandLine::PrintVersion() {
    std::cout << "Wildbound Dedicated Server v1.0.0 (Phase 39 Foundation)\n"
              << "Protocol Version: 1 | Transport: ENet v1.3.18 | Architecture: C++20 Headless\n";
}

} // namespace wildbound::server


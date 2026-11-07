#include "DedicatedServer.hpp"
#include "ServerCommandLine.hpp"
#include "../core/Logger.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
    wildbound::server::DedicatedServer server;

    if (!server.Initialize(argc, argv)) {
        if (server.GetCommandLine().ShouldShowHelp()) {
            wildbound::server::ServerCommandLine::PrintHelp();
            return 0;
        }
        if (server.GetCommandLine().ShouldShowVersion()) {
            wildbound::server::ServerCommandLine::PrintVersion();
            return 0;
        }
        std::cerr << "Dedicated server initialization failed. Exiting.\n";
        return 1;
    }

    if (server.GetCommandLine().ShouldShowHelp()) {
        wildbound::server::ServerCommandLine::PrintHelp();
        return 0;
    }
    if (server.GetCommandLine().ShouldShowVersion()) {
        wildbound::server::ServerCommandLine::PrintVersion();
        return 0;
    }

    server.Run();

    return 0;
}


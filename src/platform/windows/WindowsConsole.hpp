#pragma once

#include "../Platform.hpp"

namespace wildbound::platform::windows {

class WindowsConsole {
public:
    static bool RegisterTerminationHandler(TerminationSignalHandler handler);
    static void UnregisterTerminationHandler();
    static void ConfigureConsole();
};

} // namespace wildbound::platform::windows


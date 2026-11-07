#pragma once

#include "../Platform.hpp"

namespace wildbound::platform::linux_os {

class LinuxSignals {
public:
    static bool RegisterTerminationHandler(TerminationSignalHandler handler);
    static void UnregisterTerminationHandler();
};

} // namespace wildbound::platform::linux_os


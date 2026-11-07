#pragma once

#include <cstdint>
#include <string_view>

namespace wildbound::network {

/**
 * Lifecycle states of a network connection.
 */
enum class ConnectionState : uint8_t {
    Disconnected,
    Connecting,
    Connected,
    Disconnecting,
    TimedOut,
    Failed
};

[[nodiscard]] constexpr std::string_view ToString(ConnectionState state) noexcept {
    switch (state) {
        case ConnectionState::Disconnected:  return "Disconnected";
        case ConnectionState::Connecting:    return "Connecting";
        case ConnectionState::Connected:     return "Connected";
        case ConnectionState::Disconnecting: return "Disconnecting";
        case ConnectionState::TimedOut:      return "TimedOut";
        case ConnectionState::Failed:        return "Failed";
        default:                             return "Unknown";
    }
}

} // namespace wildbound::network


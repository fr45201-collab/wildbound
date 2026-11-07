#pragma once

#include "NetworkTypes.hpp"
#include "ConnectionState.hpp"
#include <string>
#include <vector>

namespace wildbound::network {

enum class ConnectionEventType : uint8_t {
    None,
    ClientConnected,
    ClientDisconnected,
    ConnectionAttemptFailed,
    ConnectionTimedOut,
    ConnectionRejected,
    PacketReceived,
    TransportError
};

struct ConnectionEvent {
    ConnectionEventType type{ ConnectionEventType::None };
    ClientId connectionId{ 0 };
    std::string remoteAddress;
    uint16_t remotePort{ 0 };
    ChannelType channel{ ChannelType::Control };
    uint16_t messageType{ 0 };
    std::vector<uint8_t> payload;
    std::string reason;
};

} // namespace wildbound::network


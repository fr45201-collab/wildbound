#pragma once

#include "NetworkTypes.hpp"
#include <vector>
#include <string>
#include <cstddef>

namespace wildbound::network {

struct TransportEvent {
    enum class Type : uint8_t {
        None,
        Connect,
        Disconnect,
        Data
    } type{ Type::None };

    uint16_t peerId{ 0 };
    ChannelType channel{ ChannelType::Control };
    std::vector<uint8_t> data;
};

/**
 * Pure interface for abstracting raw socket transport operations.
 * Decouples gameplay systems from third-party networking libraries (like ENet).
 */
class INetworkTransport {
public:
    virtual ~INetworkTransport() = default;

    virtual bool Initialize(uint16_t port = 0, size_t maxPeers = 32) = 0;
    virtual void Shutdown() = 0;

    virtual bool Connect(const std::string& address, uint16_t port) = 0;
    virtual void Disconnect(uint16_t peerId, uint32_t reason = 0) = 0;

    virtual bool Send(uint16_t peerId, ChannelType channel,
                      DeliveryMode mode, const uint8_t* data, size_t size) = 0;

    virtual bool PollEvent(TransportEvent& outEvent) = 0;
    virtual void Flush() = 0;

    [[nodiscard]] virtual bool IsActive() const noexcept = 0;
    [[nodiscard]] virtual size_t GetConnectedPeerCount() const noexcept = 0;
};

} // namespace wildbound::network


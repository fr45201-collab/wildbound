#pragma once

#include "INetworkTransport.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOGDI
#define NOGDI
#endif
#endif

#include <enet/enet.h>
#include <unordered_map>
#include <memory>

namespace wildbound::network {

/**
 * Concrete implementation of INetworkTransport backed by ENet.
 */
class ENetTransport : public INetworkTransport {
public:
    ENetTransport();
    ~ENetTransport() override;

    // Global ENet runtime lifecycle management
    static bool InitializeENet();
    static void DeinitializeENet();
    [[nodiscard]] static bool IsENetInitialized() noexcept;

    // INetworkTransport interface implementation
    bool Initialize(uint16_t port = 0, size_t maxPeers = 32) override;
    void Shutdown() override;

    bool Connect(const std::string& address, uint16_t port) override;
    void Disconnect(uint16_t peerId, uint32_t reason = 0) override;

    bool Send(uint16_t peerId, ChannelType channel,
              DeliveryMode mode, const uint8_t* data, size_t size) override;

    bool PollEvent(TransportEvent& outEvent) override;
    void Flush() override;

    [[nodiscard]] bool IsActive() const noexcept override { return m_host != nullptr; }
    [[nodiscard]] size_t GetConnectedPeerCount() const noexcept override { return m_peers.size(); }

    [[nodiscard]] ENetHost* GetRawHost() noexcept { return m_host; }
    [[nodiscard]] ENetPeer* GetPeer(uint16_t peerId) noexcept;

private:
    ENetHost* m_host{ nullptr };
    std::unordered_map<uint16_t, ENetPeer*> m_peers;
    std::unordered_map<ENetPeer*, uint16_t> m_peerToId;
    uint16_t m_nextPeerId{ 1 };
    bool m_isServer{ false };
};

} // namespace wildbound::network


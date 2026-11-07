#include "ENetTransport.hpp"
#include <iostream>
#include <atomic>

namespace wildbound::network {

static std::atomic<int> s_enetInitRefCount{ 0 };

bool ENetTransport::InitializeENet() {
    if (s_enetInitRefCount.fetch_add(1) == 0) {
        if (enet_initialize() != 0) {
            s_enetInitRefCount.store(0);
            return false;
        }
    }
    return true;
}

void ENetTransport::DeinitializeENet() {
    int prev = s_enetInitRefCount.fetch_sub(1);
    if (prev == 1) {
        enet_deinitialize();
    } else if (prev < 1) {
        s_enetInitRefCount.store(0);
    }
}

bool ENetTransport::IsENetInitialized() noexcept {
    return s_enetInitRefCount.load() > 0;
}

ENetTransport::ENetTransport() {
    InitializeENet();
}

ENetTransport::~ENetTransport() {
    Shutdown();
    DeinitializeENet();
}

bool ENetTransport::Initialize(uint16_t port, size_t maxPeers) {
    if (m_host) {
        Shutdown();
    }

    if (!IsENetInitialized()) {
        if (!InitializeENet()) return false;
    }

    ENetAddress address;
    m_isServer = (port > 0);

    if (m_isServer) {
        address.host = ENET_HOST_ANY;
        address.port = port;
        // Server host with specified maxPeers and 5 channels
        m_host = enet_host_create(&address, maxPeers, static_cast<size_t>(ChannelType::Count), 0, 0);
    } else {
        // Client host with 1 outgoing peer and 5 channels
        m_host = enet_host_create(nullptr, 1, static_cast<size_t>(ChannelType::Count), 0, 0);
    }

    m_peers.clear();
    m_peerToId.clear();
    m_nextPeerId = 1;

    return m_host != nullptr;
}

void ENetTransport::Shutdown() {
    if (!m_host) return;

    // Disconnect all peers immediately and flush
    for (auto& [id, peer] : m_peers) {
        if (peer) {
            enet_peer_disconnect_now(peer, 0);
        }
    }
    m_peers.clear();
    m_peerToId.clear();

    enet_host_destroy(m_host);
    m_host = nullptr;
}

bool ENetTransport::Connect(const std::string& address, uint16_t port) {
    if (!m_host || m_isServer) return false;

    ENetAddress enetAddr;
    if (enet_address_set_host(&enetAddr, address.c_str()) != 0) {
        return false;
    }
    enetAddr.port = port;

    ENetPeer* peer = enet_host_connect(m_host, &enetAddr, static_cast<size_t>(ChannelType::Count), 0);
    if (!peer) {
        return false;
    }

    m_peers[1] = peer;
    m_peerToId[peer] = 1;

    return true;
}

void ENetTransport::Disconnect(uint16_t peerId, uint32_t reason) {
    auto* peer = GetPeer(peerId);
    if (peer) {
        enet_peer_disconnect(peer, reason);
    }
}

bool ENetTransport::Send(uint16_t peerId, ChannelType channel,
                         DeliveryMode mode, const uint8_t* data, size_t size) {
    if (!m_host || !data || size == 0) return false;

    auto* peer = GetPeer(peerId);
    if (!peer) return false;

    enet_uint32 flags = 0;
    if (mode == DeliveryMode::ReliableOrdered) {
        flags |= ENET_PACKET_FLAG_RELIABLE;
    }
    // Note: in ENet, unflagged packets on a channel are unreliable sequenced by default

    ENetPacket* packet = enet_packet_create(data, size, flags);
    if (!packet) return false;

    enet_uint8 ch = static_cast<enet_uint8>(channel);
    if (ch >= static_cast<enet_uint8>(ChannelType::Count)) {
        ch = 0;
    }

    return enet_peer_send(peer, ch, packet) == 0;
}

bool ENetTransport::PollEvent(TransportEvent& outEvent) {
    outEvent.type = TransportEvent::Type::None;
    if (!m_host) return false;

    ENetEvent event;
    int res = enet_host_service(m_host, &event, 0);
    if (res <= 0) return false;

    uint16_t peerId = 0;
    auto it = m_peerToId.find(event.peer);
    if (it != m_peerToId.end()) {
        peerId = it->second;
    } else if (event.type == ENET_EVENT_TYPE_CONNECT) {
        peerId = m_nextPeerId++;
        if (m_nextPeerId == 0) m_nextPeerId = 1;
        m_peers[peerId] = event.peer;
        m_peerToId[event.peer] = peerId;
    }

    outEvent.peerId = peerId;
    outEvent.channel = static_cast<ChannelType>(event.channelID);

    switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            outEvent.type = TransportEvent::Type::Connect;
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            outEvent.type = TransportEvent::Type::Data;
            if (event.packet && event.packet->data && event.packet->dataLength > 0) {
                outEvent.data.assign(event.packet->data, event.packet->data + event.packet->dataLength);
            }
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            outEvent.type = TransportEvent::Type::Disconnect;
            if (event.peer) {
                m_peers.erase(peerId);
                m_peerToId.erase(event.peer);
            }
            break;

        default:
            break;
    }

    return true;
}

void ENetTransport::Flush() {
    if (m_host) {
        enet_host_flush(m_host);
    }
}

ENetPeer* ENetTransport::GetPeer(uint16_t peerId) noexcept {
    auto it = m_peers.find(peerId);
    return (it != m_peers.end()) ? it->second : nullptr;
}

} // namespace wildbound::network


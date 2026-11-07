#include "ENetServer.hpp"
#include <iostream>

namespace wildbound::network {

ENetServer::ENetServer() = default;

ENetServer::~ENetServer() {
    Stop();
}

bool ENetServer::Start(uint16_t port, size_t maxClients) {
    if (m_isRunning) {
        Stop();
    }

    if (!m_transport.Initialize(port, maxClients)) {
        return false;
    }

    m_port = port;
    m_isRunning = true;
    m_connectionManager.Reset();
    m_globalStats.Reset();

    return true;
}

void ENetServer::Stop() {
    if (!m_isRunning) return;

    // Disconnect all clients gracefully
    auto activeIds = m_connectionManager.GetActiveConnectionIds();
    for (ClientId id : activeIds) {
        DisconnectClient(id, 0);
    }
    m_transport.Flush();

    m_transport.Shutdown();
    m_connectionManager.Reset();
    m_isRunning = false;
    m_port = 0;
}

void ENetServer::Update(float dt) {
    if (!m_isRunning) return;

    m_connectionManager.Update(dt);
    m_globalStats.Update(dt);

    TransportEvent evt;
    while (m_transport.PollEvent(evt)) {
        switch (evt.type) {
            case TransportEvent::Type::Connect: {
                ClientId cid = m_connectionManager.AddConnection(
                    m_transport.GetPeer(evt.peerId), "remote", 0);
                m_connectionManager.SetConnectionState(cid, ConnectionState::Connected);

                if (m_eventCallback) {
                    ConnectionEvent ce;
                    ce.type = ConnectionEventType::ClientConnected;
                    ce.connectionId = cid;
                    m_eventCallback(ce);
                }
                break;
            }

            case TransportEvent::Type::Data: {
                ClientId cid = evt.peerId;
                m_globalStats.RecordReceived(evt.data.size());

                auto* conn = m_connectionManager.FindConnection(cid);
                if (conn) {
                    conn->GetStats().RecordReceived(evt.data.size());

                    // Read RTT from ENetPeer if available
                    auto* peer = m_transport.GetPeer(evt.peerId);
                    if (peer) {
                        conn->GetStats().UpdateMetrics(peer->roundTripTime, peer->packetLoss / 1000.0f);
                    }
                }

                PacketHeader header;
                auto valRes = PacketValidator::Validate(
                    evt.data.data(), evt.data.size(), static_cast<uint8_t>(evt.channel), header);

                if (valRes == PacketValidationResult::Valid) {
                    if (m_eventCallback) {
                        ConnectionEvent ce;
                        ce.type = ConnectionEventType::PacketReceived;
                        ce.connectionId = cid;
                        ce.channel = evt.channel;
                        ce.messageType = header.messageType;
                        ce.payload.assign(
                            evt.data.begin() + PACKET_HEADER_SIZE, evt.data.end());
                        m_eventCallback(ce);
                    }
                } else {
                    m_globalStats.RecordValidationFailure();
                    if (conn) {
                        conn->GetStats().RecordValidationFailure();
                    }
                }
                break;
            }

            case TransportEvent::Type::Disconnect: {
                ClientId cid = evt.peerId;
                m_connectionManager.RemoveConnection(cid);

                if (m_eventCallback) {
                    ConnectionEvent ce;
                    ce.type = ConnectionEventType::ClientDisconnected;
                    ce.connectionId = cid;
                    m_eventCallback(ce);
                }
                break;
            }

            default:
                break;
        }
    }
}

bool ENetServer::Send(ClientId clientId, ChannelType channel, DeliveryMode mode,
                      const uint8_t* data, size_t size) {
    if (!m_isRunning || !data || size == 0) return false;

    bool reliable = (mode == DeliveryMode::ReliableOrdered);
    m_globalStats.RecordSent(size, reliable);

    auto* conn = m_connectionManager.FindConnection(clientId);
    if (conn) {
        conn->GetStats().RecordSent(size, reliable);
    }

    return m_transport.Send(clientId, channel, mode, data, size);
}

void ENetServer::Broadcast(ChannelType channel, DeliveryMode mode,
                           const uint8_t* data, size_t size, ClientId excludeClientId) {
    if (!m_isRunning || !data || size == 0) return;

    for (ClientId cid : m_connectionManager.GetActiveConnectionIds()) {
        if (cid != excludeClientId) {
            Send(cid, channel, mode, data, size);
        }
    }
}

void ENetServer::DisconnectClient(ClientId clientId, uint32_t reason) {
    if (!m_isRunning) return;
    m_transport.Disconnect(clientId, reason);
    m_connectionManager.RemoveConnection(clientId);
}

} // namespace wildbound::network


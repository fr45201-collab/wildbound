#include "ENetClient.hpp"
#include <iostream>

namespace wildbound::network {

ENetClient::ENetClient() = default;

ENetClient::~ENetClient() {
    Disconnect();
}

bool ENetClient::Connect(const std::string& address, uint16_t port, uint32_t timeoutMs) {
    if (m_state != ConnectionState::Disconnected) {
        Disconnect();
    }

    if (!m_transport.Initialize(0, 1)) {
        m_state = ConnectionState::Failed;
        return false;
    }

    if (!m_transport.Connect(address, port)) {
        m_transport.Shutdown();
        m_state = ConnectionState::Failed;
        return false;
    }

    m_serverAddress = address;
    m_serverPort = port;
    m_state = ConnectionState::Connecting;
    m_connectionTimer = 0.0f;
    m_timeoutThreshold = static_cast<float>(timeoutMs) / 1000.0f;
    m_stats.Reset();

    return true;
}

void ENetClient::Disconnect(uint32_t reason) {
    if (m_state == ConnectionState::Disconnected) return;

    if (m_state == ConnectionState::Connected) {
        m_transport.Disconnect(1, reason);
        m_transport.Flush();
    }

    m_transport.Shutdown();
    m_state = ConnectionState::Disconnected;
}

void ENetClient::Update(float dt) {
    if (m_state == ConnectionState::Disconnected) return;

    m_stats.Update(dt);

    if (m_state == ConnectionState::Connecting) {
        m_connectionTimer += dt;
        if (m_connectionTimer >= m_timeoutThreshold) {
            m_state = ConnectionState::TimedOut;
            m_transport.Shutdown();

            if (m_eventCallback) {
                ConnectionEvent ce;
                ce.type = ConnectionEventType::ConnectionTimedOut;
                ce.reason = "Connection attempt timed out";
                m_eventCallback(ce);
            }
            return;
        }
    }

    // Update RTT from ENetPeer
    auto* peer = m_transport.GetPeer(1);
    if (peer && m_state == ConnectionState::Connected) {
        m_stats.UpdateMetrics(peer->roundTripTime, peer->packetLoss / 1000.0f);
    }

    TransportEvent evt;
    while (m_transport.PollEvent(evt)) {
        switch (evt.type) {
            case TransportEvent::Type::Connect: {
                m_state = ConnectionState::Connected;
                if (m_eventCallback) {
                    ConnectionEvent ce;
                    ce.type = ConnectionEventType::ClientConnected;
                    ce.connectionId = 1;
                    m_eventCallback(ce);
                }
                break;
            }

            case TransportEvent::Type::Data: {
                m_stats.RecordReceived(evt.data.size());

                PacketHeader header;
                auto valRes = PacketValidator::Validate(
                    evt.data.data(), evt.data.size(), static_cast<uint8_t>(evt.channel), header);

                if (valRes == PacketValidationResult::Valid) {
                    if (m_eventCallback) {
                        ConnectionEvent ce;
                        ce.type = ConnectionEventType::PacketReceived;
                        ce.connectionId = 1;
                        ce.channel = evt.channel;
                        ce.messageType = header.messageType;
                        ce.payload.assign(
                            evt.data.begin() + PACKET_HEADER_SIZE, evt.data.end());
                        m_eventCallback(ce);
                    }
                } else {
                    m_stats.RecordValidationFailure();
                }
                break;
            }

            case TransportEvent::Type::Disconnect: {
                m_state = ConnectionState::Disconnected;
                if (m_eventCallback) {
                    ConnectionEvent ce;
                    ce.type = ConnectionEventType::ClientDisconnected;
                    ce.connectionId = 1;
                    m_eventCallback(ce);
                }
                break;
            }

            default:
                break;
        }
    }
}

bool ENetClient::Send(ChannelType channel, DeliveryMode mode, const uint8_t* data, size_t size) {
    if (m_state != ConnectionState::Connected || !data || size == 0) return false;

    bool reliable = (mode == DeliveryMode::ReliableOrdered);
    m_stats.RecordSent(size, reliable);

    return m_transport.Send(1, channel, mode, data, size);
}

uint32_t ENetClient::GetRoundTripTimeMs() const noexcept {
    return m_stats.GetMetrics().roundTripTimeMs;
}

} // namespace wildbound::network


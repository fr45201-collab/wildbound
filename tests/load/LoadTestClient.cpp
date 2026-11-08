#include "LoadTestClient.hpp"
#include "../../src/core/Logger.hpp"
#include <cmath>
#include <iostream>

namespace wildbound::testing {

LoadTestClient::LoadTestClient(uint32_t simulatedIndex)
    : m_simulatedIndex(simulatedIndex),
      m_rng(1337 + simulatedIndex)
{
    m_simulatedAngle = static_cast<float>(simulatedIndex) * 0.5f;
    m_simulatedPosX = 300.0f + (simulatedIndex % 8) * 40.0f;
    m_simulatedPosY = 300.0f + (simulatedIndex / 8) * 40.0f;

    m_client.SetEventCallback([this](const network::ConnectionEvent& ev) {
        if (ev.type == network::ConnectionEventType::ClientConnected) {
            auto now = std::chrono::steady_clock::now();
            m_metrics.connectionTimeMs = std::chrono::duration<double, std::milli>(now - m_connectStartTime).count();
            m_metrics.connected = true;

            // Send ClientHello immediately
            network::MsgClientHello hello{
                .protocolVersion = network::PROTOCOL_VERSION,
                .playerName = m_playerName,
                .authToken = "load_test_token_" + std::to_string(m_simulatedIndex)
            };

            network::ByteStreamWriter writer;
            hello.Serialize(writer);
            SendRawPacket(network::ChannelType::Control, network::DeliveryMode::ReliableOrdered,
                          network::MessageType::ClientHello, writer.GetData(), writer.GetSize());
        } else if (ev.type == network::ConnectionEventType::PacketReceived) {
            m_metrics.packetsReceived++;
            HandlePacket(ev.messageType, ev.payload.data(), ev.payload.size());
        } else if (ev.type == network::ConnectionEventType::ClientDisconnected) {
            m_metrics.connected = false;
        }
    });
}

LoadTestClient::~LoadTestClient() {
    Disconnect();
}

bool LoadTestClient::Connect(const std::string& host, uint16_t port, const std::string& playerName) {
    m_playerName = playerName;
    m_metrics.playerName = playerName;
    m_metrics.clientId = m_simulatedIndex;
    m_connectStartTime = std::chrono::steady_clock::now();
    return m_client.Connect(host, port);
}

void LoadTestClient::Disconnect() {
    if (m_client.IsConnected()) {
        m_client.Disconnect();
    }
    m_isAccepted = false;
}

void LoadTestClient::SendRawPacket(network::ChannelType channel, network::DeliveryMode mode,
                                  network::MessageType type, const uint8_t* payload, size_t size)
{
    if (m_artificialLossPercent > 0.0) {
        std::uniform_real_distribution<double> dist(0.0, 100.0);
        if (dist(m_rng) < m_artificialLossPercent) {
            // Drop packet artificially to simulate adverse conditions
            return;
        }
    }

    network::PacketHeader hdr{
        .magic = network::PROTOCOL_MAGIC,
        .version = network::PROTOCOL_VERSION,
        .messageType = static_cast<uint16_t>(type),
        .sequenceNumber = static_cast<network::SequenceNumber>(++m_inputSequence),
        .senderClientId = m_assignedClientId,
        .payloadLength = static_cast<uint16_t>(size)
    };

    network::ByteStreamWriter pw;
    network::SerializePacketHeader(pw, hdr);
    if (size > 0 && payload) {
        pw.WriteBytes(payload, size);
    }

    if (m_client.Send(channel, mode, pw.GetData(), pw.GetSize())) {
        m_metrics.packetsSent++;
    }
}

void LoadTestClient::HandlePacket(uint16_t messageType, const uint8_t* data, size_t size) {
    network::ByteStreamReader reader(data, size);

    if (messageType == static_cast<uint16_t>(network::MessageType::ConnectionAccepted)) {
        network::MsgConnectionAccepted acc;
        if (acc.Deserialize(reader)) {
            m_isAccepted = true;
            m_metrics.accepted = true;
            m_assignedClientId = acc.assignedClientId;
            m_assignedParticipantId = acc.assignedPlayerId.id.value;
            m_assignedMatchId = acc.matchId;
        }
    } else if (messageType == static_cast<uint16_t>(network::MessageType::StateSnapshot)) {
        m_metrics.snapshotsReceived++;
    }
}

void LoadTestClient::Update(float dt) {
    m_client.Update(dt);
    m_metrics.rttMs = m_client.GetRoundTripTimeMs();

    if (!m_isAccepted || !m_autonomousSimulation) {
        return;
    }

    // 1. Fixed 30 Hz input simulation
    m_inputTimer += dt;
    if (m_inputTimer >= (1.0f / 30.0f)) {
        m_inputTimer -= (1.0f / 30.0f);

        // Circular wandering motion
        m_simulatedAngle += 0.05f;
        float dirX = std::cos(m_simulatedAngle);
        float dirY = std::sin(m_simulatedAngle);

        SendInput(dirX, dirY, m_simulatedAngle, 0);
    }

    // 2. Periodic attack simulation (every 2.5 seconds)
    m_attackTimer += dt;
    if (m_attackTimer >= 2.5f) {
        m_attackTimer = 0.0f;
        SendAttack(1, m_simulatedAngle);
    }
}

void LoadTestClient::SendInput(float dirX, float dirY, float facingAngle, uint8_t actionFlags) {
    network::MsgPlayerInput input{
        .inputSequence = ++m_inputSequence,
        .dirX = dirX,
        .dirY = dirY,
        .facingAngle = facingAngle,
        .actionFlags = actionFlags
    };

    network::ByteStreamWriter writer;
    input.Serialize(writer);
    SendRawPacket(network::ChannelType::PlayerInput, network::DeliveryMode::UnreliableSequenced,
                  network::MessageType::PlayerInput, writer.GetData(), writer.GetSize());
}

void LoadTestClient::SendAttack(uint8_t weaponSlot, float aimAngle) {
    network::MsgAttackRequest attack{
        .weaponSlot = weaponSlot,
        .aimAngle = aimAngle,
        .clientTick = m_inputSequence
    };

    network::ByteStreamWriter writer;
    attack.Serialize(writer);
    SendRawPacket(network::ChannelType::ReliableEvents, network::DeliveryMode::ReliableOrdered,
                  network::MessageType::AttackRequest, writer.GetData(), writer.GetSize());
    m_metrics.attacksSent++;
}

void LoadTestClient::SendPickup(uint32_t itemId) {
    network::MsgPickupRequest pickup{
        .targetItemId = network::NetworkEntityIdentity{
            .id = network::NetworkEntityId{ itemId },
            .generation = 1
        }
    };

    network::ByteStreamWriter writer;
    pickup.Serialize(writer);
    SendRawPacket(network::ChannelType::ReliableEvents, network::DeliveryMode::ReliableOrdered,
                  network::MessageType::PickupRequest, writer.GetData(), writer.GetSize());
    m_metrics.pickupsSent++;
}

} // namespace wildbound::testing

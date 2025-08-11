#include "tcp_echo/net/Connection.h"

#include "tcp_echo/util/ByteOrder.hpp"

#include <stdexcept>

namespace tcp_echo::net
{
    bool Connection::OnRead()
    {
        unsigned char tmp[4096];
        // Read available frames
        try
        {
            std::size_t numRecv = m_sock.RecvSome(tmp, sizeof(tmp));
            if (numRecv == 0)
            {
                LOG_INFO("conn", "peer closed connection (" + Who() + ")");
                m_state = State::Closed;
                return false; // Peer closed connection
            }
            m_inbuf.insert(m_inbuf.end(), tmp, tmp + numRecv);
            m_lastActivity = std::chrono::steady_clock::now();
        }
        catch (const std::exception& e)
        {
            LOG_WARN("conn", std::string("recv failed (" + Who() + "): " + e.what()));
            return false;
        }

        // Extract and handle as many frames as are available
        try
        {
            for (auto frame = tcp_echo::proto::TryExtractFrame(m_inbuf, m_maxFrame);
                not frame.empty();
                frame = tcp_echo::proto::TryExtractFrame(m_inbuf, m_maxFrame))
            {
                if (not HandleFrame(frame)) return false;
            }
        }
        catch (const std::exception& e)
        {
            LOG_WARN("conn", std::string("frame extraction failed (") + e.what() + ")");
            return false;
        }

        return true;
    }

    void Connection::OnTick(int idleTimeoutMs)
    {
        if (idleTimeoutMs <= 0) return;
        const auto now = std::chrono::steady_clock::now();
        const auto idleDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastActivity).count();
        if (idleDuration > idleTimeoutMs)
        {
            LOG_INFO("conn", "closing due to inactivity (" + Who()
                     + "), idle for " + std::to_string(idleDuration) + " ms");
            m_state = State::Closed;
            m_sock.Close();
        }
    }

    bool Connection::HandleFrame(const std::vector<uint8_t>& frame)
    {
        if (frame.size() < tcp_echo::proto::HEADER_SIZE) return false;
        auto type = static_cast<tcp_echo::proto::MsgType>(frame[2]);
        switch (type)
        {
            case tcp_echo::proto::MsgType::LoginReq:
                return HandleLogin(frame);
            case tcp_echo::proto::MsgType::EchoReq:
                return HandleEcho(frame);
            default:
                LOG_WARN("conn", "unexpected message type (" + Who() + ")");
                return false;
        }
    }

    bool Connection::HandleLogin(const std::vector<uint8_t>& frame)
    {
        if (m_state != State::WaitingLogin) return false;

        tcp_echo::proto::LoginRequest req{};
        if (not tcp_echo::proto::Deserialize(frame, req)) return false;

        m_username = req.username;
        m_password = req.password;
        userSum = tcp_echo::cipher::sum_u8(m_username);
        passSum = tcp_echo::cipher::sum_u8(m_password);

        // In this assignment any username/password is valid
        tcp_echo::proto::LoginResponse resp{};
        resp.status = 1; // OK
        resp.seq = req.seq;

        auto bytes = tcp_echo::proto::Serialize(resp);
        if (not SendBytes(bytes)) return false;

        m_state = State::LoggedIn;
        LOG_INFO("conn", "login OK for user '" + m_username + "' (" + Who() + ")");

        return true;
    }

    bool Connection::HandleEcho(const std::vector<uint8_t>& frame)
    {
        if (m_state != State::LoggedIn) return false;

        tcp_echo::proto::EchoRequest req{};
        if (not tcp_echo::proto::Deserialize(frame, req)) return false;

        // Build keystream from (seq, username_sum, password_sum)
        const uint32_t init = tcp_echo::cipher::MakeInitialKey(req.seq, userSum, passSum);
        auto ks = tcp_echo::cipher::Keystream(init, req.msgSize);

        // Decrypt
        std::vector<uint8_t> plain = req.cipher;
        tcp_echo::cipher::InplaceXOR(plain, ks);

        tcp_echo::proto::EchoResponse resp{};
        resp.seq = req.seq;
        resp.msgSize = static_cast<uint16_t>(plain.size());
        resp.plain = std::move(plain);

        auto bytes = tcp_echo::proto::Serialize(resp);
        if (not SendBytes(bytes)) return false;

        LOG_DEBUG("conn", "echo handled (" + Who() + "), bytes=" + std::to_string(resp.msgSize));
        return true;
    }

    bool Connection::SendBytes(const std::vector<uint8_t>& bytes)
    {
        try
        {
            (void)m_sock.SendAll(bytes.data(), bytes.size());
            return true;
        }
        catch (const std::exception& e)
        {
            LOG_WARN("conn", std::string("send failed: ") + Who() + "): " + e.what());
            return false;
        }
    }
} // namespace tcp_echo::net

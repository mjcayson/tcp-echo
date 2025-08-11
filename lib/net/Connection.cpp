#include "tcp_echo/net/Connection.h"
#include "tcp_echo/util/ByteOrder.hpp"
#include <stdexcept>

namespace tcp_echo::net
{
    bool Connection::OnRead()
    {
        unsigned char tmp[4096];
        // Read what's available
        try
        {
            std::size_t n = m_sock.RecvSome(tmp, sizeof(tmp));
            if (n == 0) return false; // peer closed
            m_inbuf.insert(m_inbuf.end(), tmp, tmp + n);
        }
        catch (const std::exception&)
        {
            return false; // treat errors as fatal for simplicity
        }

        // Extract and handle as many frames as are available
        for (auto frame = tcp_echo::proto::TryExtractFrame(m_inbuf);
             not frame.empty();
             frame = tcp_echo::proto::TryExtractFrame(m_inbuf))
        {
            if (not HandleFrame(frame)) return false;
        }

        return true;
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
                LOG_WARN("conn", "unexpected message type");
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
        LOG_INFO("conn", "login OK for user '" + m_username + "'");

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
        return SendBytes(bytes);
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
            LOG_WARN("conn", std::string("send failed: ") + e.what());
            return false;
        }
    }
} // namespace tcp_echo::net

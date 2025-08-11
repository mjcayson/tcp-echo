#pragma once

#include "tcp_echo/net/Socket.h"
#include "tcp_echo/proto/FrameCodec.h"
#include "tcp_echo/proto/Messages.h"
#include "tcp_echo/util/Cipher.hpp"
#include "tcp_echo/util/Logger.hpp"

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace tcp_echo::net
{
  class Connection
  {
  public:
    enum class State { WaitingLogin, LoggedIn, Closed };

    Connection(Socket sock,
               std::size_t maxFrame,
               std::string peerIp = {},
               uint16_t peerPort = 0)
      : m_sock(std::move(sock)),
        m_maxFrame(maxFrame),
        m_peerIp(std::move(peerIp)),
        m_peerPort(peerPort)
      {}

    int fd() const { return m_sock.fd(); }
    State GetState() const { return m_state; }

    // Called when fd is readable. Returns false if connection should be closed.
    bool OnRead();
    //Tick hook for idle timeout. Will close() socket on idle to break the loop.
    void OnTick(int idleTimeoutMs);

  private:
    bool HandleFrame(const std::vector<uint8_t>& frame);
    bool HandleLogin(const std::vector<uint8_t>& frame);
    bool HandleEcho(const std::vector<uint8_t>& frame);
    bool SendBytes(const std::vector<uint8_t>& bytes);

    std::string Who() const
    {
      return m_peerIp.empty()
        ? ("fd=" + std::to_string(fd()))
        : (m_peerIp + ":" + std::to_string(m_peerPort) + " fd=" + std::to_string(fd()));
    }

  private:
    Socket m_sock;
    std::vector<uint8_t> m_inbuf;
    State m_state{State::WaitingLogin};
    //successful login
    std::string m_username;
    std::string m_password;
    uint8_t userSum{0};
    uint8_t passSum{0};
    std::size_t m_maxFrame{8192};
    std::chrono::steady_clock::time_point m_lastActivity{std::chrono::steady_clock::now()};
    std::string m_peerIp;
    uint16_t m_peerPort{0};
  };
} // namespace tcp_echo::net

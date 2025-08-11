#pragma once

#include "tcp_echo/net/Socket.h"
#include "tcp_echo/proto/FrameCodec.h"
#include "tcp_echo/proto/Messages.h"
#include "tcp_echo/util/Cipher.hpp"
#include "tcp_echo/util/Logger.hpp"

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

    explicit Connection(Socket s)
      : m_sock(std::move(s)) {}

    int fd() const { return m_sock.fd(); }
    State GetState() const { return m_state; }

    // Called when fd is readable. Returns false if connection should be closed.
    bool OnRead();

  private:
    bool HandleFrame(const std::vector<uint8_t>& frame);

    bool HandleLogin(const std::vector<uint8_t>& frame);
    bool HandleEcho(const std::vector<uint8_t>& frame);

    bool SendBytes(const std::vector<uint8_t>& bytes);

  private:
    Socket m_sock;
    std::vector<uint8_t> m_inbuf;
    State m_state{State::WaitingLogin};
    // After successful login
    std::string m_username;
    std::string m_password;
    uint8_t userSum{0};
    uint8_t passSum{0};
  };
} // namespace tcp_echo::net

#pragma once
#include <cstdint>
#include <string>
#include "tcp_echo/net/Socket.h"

namespace tcp_echo::net
{
  class Acceptor
  {
  public:
    Acceptor(const std::string& host, uint16_t port);
    int fd() const { return m_sock.fd(); }
    Socket AcceptOne(std::string* ip=nullptr, uint16_t* port=nullptr);

  private:
    Socket m_sock;
  };
} // namespace tcp_echo::net

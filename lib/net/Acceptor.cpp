#include "tcp_echo/net/Acceptor.h"

namespace tcp_echo::net
{
  Acceptor::Acceptor(const std::string& host, uint16_t port)
    : m_sock(Socket::CreateTcp())
  {
    m_sock.BindAndListen(host, port, 128);
  }

  Socket Acceptor::AcceptOne(std::string* ip, uint16_t* port)
  {
    return m_sock.Accept(ip, port);
  }
} // namespace tcp_echo::net

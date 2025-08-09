#include "tcp_echo/net/acceptor.hpp"

namespace tcp_echo::net {

Acceptor::Acceptor(const std::string& host, uint16_t port)
  : sock_(Socket::create_tcp()) {
  sock_.bind_and_listen(host, port, 128);
}

Socket Acceptor::accept_one(std::string* ip, uint16_t* port) {
  return sock_.accept(ip, port);
}

} // namespace tcp_echo::net

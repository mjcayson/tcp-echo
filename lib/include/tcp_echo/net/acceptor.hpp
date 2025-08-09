#pragma once
#include <cstdint>
#include <string>
#include "tcp_echo/net/socket.hpp"

namespace tcp_echo::net {

class Acceptor {
public:
  Acceptor(const std::string& host, uint16_t port);
  int fd() const { return sock_.fd(); }
  Socket accept_one(std::string* ip=nullptr, uint16_t* port=nullptr);

private:
  Socket sock_;
};

} // namespace tcp_echo::net

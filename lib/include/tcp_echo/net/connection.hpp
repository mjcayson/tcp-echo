#pragma once
#include "tcp_echo/net/socket.hpp"
#include <cstdint>

namespace tcp_echo::net {

// Minimal placeholder; we'll flesh out state machine next phase.
class Connection {
public:
  explicit Connection(Socket s) : sock_(std::move(s)) {}
  int fd() const { return sock_.fd(); }

private:
  Socket sock_;
};

} // namespace tcp_echo::net

#pragma once
#include "tcp_echo/net/acceptor.hpp"
#include "tcp_echo/net/connection_manager.hpp"
#include "tcp_echo/net/reactor.hpp"
#include "tcp_echo/util/logger.hpp"
#include "tcp_echo/util/signal.hpp"
#include "tcp_echo/util/config.hpp"
#include <memory>

namespace tcp_echo::net {

class ServerCore {
public:
  explicit ServerCore(const tcp_echo::ServerConfig& cfg);
  void run();

private:
  tcp_echo::ServerConfig cfg_;
  Acceptor acceptor_;
  ConnectionManager mgr_;
  std::unique_ptr<Reactor> reactor_;
  tcp_echo::sys::StopToken stop_;
};

} // namespace tcp_echo::net

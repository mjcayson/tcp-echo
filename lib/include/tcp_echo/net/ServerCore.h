#pragma once
#include "tcp_echo/net/Acceptor.h"
#include "tcp_echo/net/ConnectionManager.hpp"
#include "tcp_echo/net/Reactor.h"
#include "tcp_echo/util/Logger.hpp"
#include "tcp_echo/util/Signal.h"
#include "tcp_echo/util/Config.h"
#include <memory>

namespace tcp_echo::net
{
  class ServerCore
  {
  public:
    explicit ServerCore(const tcp_echo::ServerConfig& cfg);
    void Run();

  private:
    tcp_echo::ServerConfig m_cfg;
    Acceptor m_acceptor;
    ConnectionManager m_mgr;
    std::unique_ptr<Reactor> m_reactor;
    tcp_echo::sys::StopToken m_stop;
  };
} // namespace tcp_echo::net

#include "tcp_echo/net/ServerCore.h"

#include <memory>
#include <utility>
#include <unistd.h>

namespace tcp_echo::net
{
  extern Reactor* MakeSelectReactor(); // from ReactorSelect.cpp

  ServerCore::ServerCore(const tcp_echo::ServerConfig& cfg)
    : m_cfg(cfg), m_acceptor(cfg.host, cfg.port)
  {
    m_reactor.reset(MakeSelectReactor());
  }

  void ServerCore::Run()
  {
    using namespace tcp_echo;

    log::SetLevel(m_cfg.logLevel);
    sys::InstallSignalHandlers(m_stop);

    LOG_INFO("server", "listening on " + m_cfg.host + ":" + std::to_string(m_cfg.port));

    // 1) accept new connections
    m_reactor->AddRead(m_acceptor.fd(), [this](int /*fd*/)
    {
      std::string ip;
      uint16_t port{};
      try
      {
        Socket sock = m_acceptor.AcceptOne(&ip, &port);
        int cfd = sock.fd();
        LOG_INFO("server", "accepted " + ip + ":" + std::to_string(port));
        m_mgr.Add(cfd, Connection(std::move(sock)));

        // Register read handler for this connection
        m_reactor->AddRead(cfd, [this, cfd](int /*fd*/)
        {
          Connection* conn = m_mgr.GetConnection(cfd);
          if (conn == nullptr) return; // already removed
          if (not conn->OnRead())
          {
            // close path
            m_reactor->Del(cfd);
            m_mgr.Remove(cfd);
            LOG_INFO("server", "connection closed fd=" + std::to_string(cfd));
          }
        });
      }
      catch (const std::exception& e)
      {
        LOG_WARN("server", std::string("accept failed: ") + e.what());
      }
    });

    // 2) run until stop (Ctrl+C / SIGTERM)
    m_reactor->Run();
    if (m_stop.StopRequested()) LOG_INFO("server", "shutdown requested");
  }
} // namespace tcp_echo::net

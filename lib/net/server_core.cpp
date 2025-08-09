#include "tcp_echo/net/server_core.hpp"
#include <memory>
#include <utility>
#include <unistd.h>

namespace tcp_echo::net {

extern Reactor* make_select_reactor(); // from reactor_select.cpp

ServerCore::ServerCore(const tcp_echo::ServerConfig& cfg)
  : cfg_(cfg), acceptor_(cfg.host, cfg.port) {
  // For Phase 1 we always use select; epoll can be added later
  reactor_.reset(make_select_reactor());
}

void ServerCore::run() {
  using namespace tcp_echo;
  log::set_level(cfg_.log_level);
  sys::install_signal_handlers(stop_);

  LOG_INFO("server", "listening on " + cfg_.host + ":" + std::to_string(cfg_.port));

  // Register acceptor
  reactor_->add_read(acceptor_.fd(), [this](int /*fd*/) {
    std::string ip; uint16_t port{};
    try {
      Socket s = acceptor_.accept_one(&ip, &port);
      int cfd = s.fd();
      LOG_INFO("server", "accepted " + ip + ":" + std::to_string(port));
      mgr_.add(cfd, Connection(std::move(s)));
      // For Phase 1 we only track the connection; message handling comes next
    } catch (const std::exception& e) {
      LOG_WARN("server", std::string("accept failed: ") + e.what());
    }
  });

  // Run until signal
  reactor_->run();
  if (stop_.stop_requested()) LOG_INFO("server", "shutdown requested");
}

} // namespace tcp_echo::net

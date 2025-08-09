#include "tcp_echo/util/config.hpp"
#include "tcp_echo/util/logger.hpp"
#include "tcp_echo/net/socket.hpp"

int main(int argc, char* argv[]) {
  auto cfg = tcp_echo::parse_client_config(argc, argv);
  tcp_echo::log::set_level(cfg.log_level);

  tcp_echo::net::Socket s = tcp_echo::net::Socket::create_tcp();
  s.connect(cfg.host, cfg.port);
  LOG_INFO("client", "connected to " + cfg.host + ":" + std::to_string(cfg.port));
  // Phase 2: send Login, then Echo
  return 0;
}

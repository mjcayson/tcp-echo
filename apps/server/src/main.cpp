#include "tcp_echo/util/config.hpp"
#include "tcp_echo/util/logger.hpp"
#include "tcp_echo/net/server_core.hpp"

int main(int argc, char* argv[]) {
  tcp_echo::ServerConfig cfg = tcp_echo::parse_server_config(argc, argv);
  tcp_echo::log::set_level(cfg.log_level);
  tcp_echo::net::ServerCore server(cfg);
  server.run();
  return 0;
}

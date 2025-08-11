#include "tcp_echo/util/Config.h"
#include "tcp_echo/util/Logger.hpp"
#include "tcp_echo/net/ServerCore.h"

int main(int argc, char* argv[]) {
  tcp_echo::ServerConfig cfg = tcp_echo::ParseServerConfig(argc, argv);
  tcp_echo::log::SetLevel(cfg.logLevel);
  tcp_echo::net::ServerCore server(cfg);
  server.Run();
  
  return 0;
}

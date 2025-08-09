#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include "tcp_echo/util/logger.hpp"

namespace tcp_echo {

struct ServerConfig {
  std::string host = "0.0.0.0";
  uint16_t    port = 5000;
  std::string reactor = "select";   // "select" | "epoll"
  int         workers = 1;          // not used yet, for future thread pool
  log::Level  log_level = log::Level::info;
};

struct ClientConfig {
  std::string host = "127.0.0.1";
  uint16_t    port = 5000;
  std::string user = "testuser";
  std::string pass = "testpass";
  std::string message = "hello";
  int         parallel = 1;
  log::Level  log_level = log::Level::info;
};

ServerConfig parse_server_config(int argc, char* argv[]);
ClientConfig parse_client_config(int argc, char* argv[]);

} // namespace tcp_echo

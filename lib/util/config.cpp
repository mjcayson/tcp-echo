#include "tcp_echo/util/config.hpp"
#include <cstdlib>
#include <cstring>

namespace {

tcp_echo::log::Level parse_level(const char* s) {
  if (!s) return tcp_echo::log::Level::info;
  std::string v(s);
  if (v == "trace") return tcp_echo::log::Level::trace;
  if (v == "debug") return tcp_echo::log::Level::debug;
  if (v == "info")  return tcp_echo::log::Level::info;
  if (v == "warn")  return tcp_echo::log::Level::warn;
  if (v == "error") return tcp_echo::log::Level::error;
  return tcp_echo::log::Level::info;
}

uint16_t to_u16(const char* s, uint16_t defv) {
  if (!s) return defv;
  long v = std::strtol(s, nullptr, 10);
  if (v < 0 || v > 65535) return defv;
  return static_cast<uint16_t>(v);
}

int to_int(const char* s, int defv) {
  if (!s) return defv;
  long v = std::strtol(s, nullptr, 10);
  return static_cast<int>(v);
}

} // namespace

namespace tcp_echo {

ServerConfig parse_server_config(int argc, char* argv[]) {
  ServerConfig cfg;
  // ENV defaults (lowest precedence)
  if (const char* h = std::getenv("ECHO_HOST")) cfg.host = h;
  if (const char* p = std::getenv("ECHO_PORT")) cfg.port = to_u16(p, cfg.port);
  if (const char* r = std::getenv("ECHO_REACTOR")) cfg.reactor = r;
  if (const char* w = std::getenv("ECHO_WORKERS")) cfg.workers = to_int(w, cfg.workers);
  if (const char* l = std::getenv("ECHO_LOG_LEVEL")) cfg.log_level = parse_level(l);

  // CLI overrides
  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    auto next = [&](const char* name) -> const char* {
      if (i + 1 >= argc) { LOG_WARN("config", std::string("missing value for ") + name); return nullptr; }
      return argv[++i];
    };
    if (a == "--host")      cfg.host = next("--host");
    else if (a == "--port") cfg.port = to_u16(next("--port"), cfg.port);
    else if (a == "--reactor") cfg.reactor = next("--reactor");
    else if (a == "--workers") cfg.workers = to_int(next("--workers"), cfg.workers);
    else if (a == "--log-level") cfg.log_level = parse_level(next("--log-level"));
    else if (a == "--help" || a == "-h") {
      std::cout <<
        "Server options:\n"
        "  --host <ip>          (default 0.0.0.0)\n"
        "  --port <num>         (default 5000)\n"
        "  --reactor <select|epoll>\n"
        "  --workers <n>\n"
        "  --log-level <trace|debug|info|warn|error>\n";
      std::exit(0);
    }
  }
  return cfg;
}

ClientConfig parse_client_config(int argc, char* argv[]) {
  ClientConfig cfg;
  if (const char* h = std::getenv("ECHO_HOST")) cfg.host = h;
  if (const char* p = std::getenv("ECHO_PORT")) cfg.port = to_u16(p, cfg.port);
  if (const char* l = std::getenv("ECHO_LOG_LEVEL")) cfg.log_level = parse_level(l);

  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    auto next = [&](const char* name) -> const char* {
      if (i + 1 >= argc) { LOG_WARN("config", std::string("missing value for ") + name); return nullptr; }
      return argv[++i];
    };
    if (a == "--host") cfg.host = next("--host");
    else if (a == "--port") cfg.port = to_u16(next("--port"), cfg.port);
    else if (a == "--user") cfg.user = next("--user");
    else if (a == "--pass") cfg.pass = next("--pass");
    else if (a == "--msg" || a == "--message") cfg.message = next("--msg");
    else if (a == "--parallel") cfg.parallel = to_int(next("--parallel"), cfg.parallel);
    else if (a == "--log-level") cfg.log_level = parse_level(next("--log-level"));
    else if (a == "--help" || a == "-h") {
      std::cout <<
        "Client options:\n"
        "  --host <ip>          (default 127.0.0.1)\n"
        "  --port <num>         (default 5000)\n"
        "  --user <u> --pass <p>\n"
        "  --msg  <text>\n"
        "  --parallel <n>\n"
        "  --log-level <trace|debug|info|warn|error>\n";
      std::exit(0);
    }
  }
  return cfg;
}

} // namespace tcp_echo

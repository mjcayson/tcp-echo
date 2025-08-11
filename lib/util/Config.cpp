#include "tcp_echo/util/Config.h"
#include <cstdlib>
#include <cstring>

namespace
{
  tcp_echo::log::Level ParseLevel(const char* lvl)
  {
    if (lvl == nullptr) return tcp_echo::log::Level::info;

    std::string val(lvl);
    if (val == "trace") return tcp_echo::log::Level::trace;
    if (val == "debug") return tcp_echo::log::Level::debug;
    if (val == "info")  return tcp_echo::log::Level::info;
    if (val == "warn")  return tcp_echo::log::Level::warn;
    if (val == "error") return tcp_echo::log::Level::error;

    return tcp_echo::log::Level::info;
  }

  uint16_t to_u16(const char* portStr, uint16_t defaultVal)
  {
    if (portStr == nullptr) return defaultVal;
    long val = std::strtol(portStr, nullptr, 10);
    if (val < 0 || val > 65535) return defaultVal;
    return static_cast<uint16_t>(val);
  }

  int to_int(const char* workers, int defaultVal)
  {
    if (workers == nullptr) return defaultVal;
    long val = std::strtol(workers, nullptr, 10);
    return static_cast<int>(val);
  }

  void PrintServerHelpThenExit()
  {
    std::cout <<
      "Server options:\n"
      "  --host <ip>          (default 0.0.0.0)\n"
      "  --port <num>         (default 5000)\n"
      "  --reactor <select|epoll>\n"
      "  --workers <n>\n"
      "  --log-level <trace|debug|info|warn|error>\n"
      "  --max-frame <bytes>   (default 8192)\n"
      "  --idle-ms <ms>        close idle connections after N ms (0=off)\n";
    
    std::exit(0);
  }

  void PrintClientHelpThenExit()
  {
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
} // namespace

namespace tcp_echo
{
  ServerConfig ParseServerConfig(int argc, char* argv[])
  {
    ServerConfig cfg;
    // ENV defaults (lowest precedence)
    if (const char* host = std::getenv("ECHO_HOST")) cfg.host = host;
    if (const char* port = std::getenv("ECHO_PORT")) cfg.port = to_u16(port, cfg.port);
    if (const char* reactor = std::getenv("ECHO_REACTOR")) cfg.reactor = reactor;
    if (const char* workers = std::getenv("ECHO_WORKERS")) cfg.workers = to_int(workers, cfg.workers);
    if (const char* lvl = std::getenv("ECHO_LOG_LEVEL")) cfg.logLevel = ParseLevel(lvl);
    if (const char* maxFrame = std::getenv("ECHO_MAX_FRAME")) cfg.maxFrame
        = static_cast<std::size_t>(to_int(maxFrame, static_cast<int>(cfg.maxFrame)));
    if (const char* timeoutMs = std::getenv("ECHO_IDLE_MS"))  cfg.idleTimeoutMs = to_int(timeoutMs, cfg.idleTimeoutMs);


    // CLI overrides
    for (int counter = 1; counter < argc; ++counter)
    {
      std::string arg = argv[counter];
      auto next = [&](const char* name) -> const char*
      {
        if (counter + 1 >= argc)
        {
          LOG_WARN("config", std::string("missing value for ") + name);
          return nullptr;
        }
        return argv[++counter];
      };
      if (arg == "--host") cfg.host = next("--host");
      else if (arg == "--port") cfg.port = to_u16(next("--port"), cfg.port);
      else if (arg == "--reactor") cfg.reactor = next("--reactor");
      else if (arg == "--workers") cfg.workers = to_int(next("--workers"), cfg.workers);
      else if (arg == "--log-level") cfg.logLevel = ParseLevel(next("--log-level"));
      else if (arg == "--max-frame") cfg.maxFrame = static_cast<std::size_t>(to_int(next("--max-frame"),
                                                                                    static_cast<int>(cfg.maxFrame)));
      else if (arg == "--idle-ms") cfg.idleTimeoutMs = to_int(next("--idle-ms"), cfg.idleTimeoutMs);
      else if (arg == "--help" || arg == "-h") PrintServerHelpThenExit();
    }
    return cfg;
  }

  ClientConfig ParseClientConfig(int argc, char* argv[])
  {
    ClientConfig cfg;
    if (const char* host = std::getenv("ECHO_HOST")) cfg.host = host;
    if (const char* port = std::getenv("ECHO_PORT")) cfg.port = to_u16(port, cfg.port);
    if (const char* lvl = std::getenv("ECHO_LOG_LEVEL")) cfg.logLevel = ParseLevel(lvl);

    for (int counter = 1; counter < argc; ++counter)
    {
      std::string arg = argv[counter];
      auto next = [&](const char* name) -> const char*
      {
        if (counter + 1 >= argc)
        {
          LOG_WARN("config", std::string("missing value for ") + name);
          return nullptr;
        }

        return argv[++counter];
      };
      if (arg == "--host") cfg.host = next("--host");
      else if (arg == "--port") cfg.port = to_u16(next("--port"), cfg.port);
      else if (arg == "--user") cfg.user = next("--user");
      else if (arg == "--pass") cfg.pass = next("--pass");
      else if (arg == "--msg" || arg == "--message") cfg.message = next("--msg");
      else if (arg == "--parallel") cfg.parallel = to_int(next("--parallel"), cfg.parallel);
      else if (arg == "--log-level") cfg.logLevel = ParseLevel(next("--log-level"));
      else if (arg == "--help" || arg == "-h") PrintClientHelpThenExit();
    }

    return cfg;
  }
} // namespace tcp_echo

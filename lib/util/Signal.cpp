#include "tcp_echo/util/Signal.h"
#include "tcp_echo/util/Logger.hpp"
#include <csignal>

namespace
{
  tcp_echo::sys::StopToken* stopToken = nullptr;

  void handler(int sig)
  {
    if (stopToken) stopToken->RequestStop();
    LOG_WARN("signal", std::string("received signal ") + std::to_string(sig));
  }
}

namespace tcp_echo::sys
{
  void InstallSignalHandlers(StopToken& token)
  {
    stopToken = &token;
    std::signal(SIGINT,  handler);
    std::signal(SIGTERM, handler);
  }
} // namespace tcp_echo::sys

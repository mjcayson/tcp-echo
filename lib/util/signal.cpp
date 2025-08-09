#include "tcp_echo/util/signal.hpp"
#include "tcp_echo/util/logger.hpp"
#include <csignal>

namespace {
tcp_echo::sys::StopToken* g_token = nullptr;

void handler(int sig) {
  if (g_token) g_token->request_stop();
  LOG_WARN("signal", std::string("received signal ") + std::to_string(sig));
}
}

namespace tcp_echo::sys {
void install_signal_handlers(StopToken& token) {
  g_token = &token;
  std::signal(SIGINT,  handler);
  std::signal(SIGTERM, handler);
}
} // namespace tcp_echo::sys

#pragma once
#include <atomic>

namespace tcp_echo::sys {

class StopToken {
public:
  void request_stop() { flag_.store(true); }
  bool stop_requested() const { return flag_.load(); }
private:
  std::atomic<bool> flag_{false};
};

void install_signal_handlers(StopToken& token);

} // namespace tcp_echo::sys

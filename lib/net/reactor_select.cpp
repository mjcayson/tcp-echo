#include "tcp_echo/net/reactor.hpp"
#include "tcp_echo/util/logger.hpp"
#include <sys/select.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace tcp_echo::net {

class SelectReactor : public Reactor {
public:
  void add_read(Fd fd, IoCallback cb) override {
    callbacks_[fd] = std::move(cb);
    if (fd > maxfd_) maxfd_ = fd;
  }
  void del(Fd fd) override {
    callbacks_.erase(fd);
    if (fd == maxfd_) recompute_max();
  }
  void run() override {
    running_ = true;
    while (running_) {
      fd_set rfds;
      FD_ZERO(&rfds);
      int local_max = -1;
      for (auto& [fd, _] : callbacks_) { FD_SET(fd, &rfds); if (fd > local_max) local_max = fd; }
      if (local_max < 0) { ::usleep(1000 * 10); continue; } // idle
      int rc = ::select(local_max + 1, &rfds, nullptr, nullptr, nullptr);
      if (rc < 0) { if (running_) LOG_WARN("reactor", "select failed; continuing"); continue; }
      std::vector<int> ready;
      ready.reserve(static_cast<std::size_t>(rc));
      for (auto& [fd, _] : callbacks_) if (FD_ISSET(fd, &rfds)) ready.push_back(fd);
      for (int fd : ready) {
        auto it = callbacks_.find(fd);
        if (it != callbacks_.end()) it->second(fd);
      }
    }
  }
  void stop() override { running_ = false; }

private:
  void recompute_max() {
    maxfd_ = -1;
    for (auto& [fd, _] : callbacks_) if (fd > maxfd_) maxfd_ = fd;
  }
  std::unordered_map<int, IoCallback> callbacks_;
  int maxfd_{-1};
  bool running_{false};
};

// factory function for now (could be moved elsewhere)
Reactor* make_select_reactor() { return new SelectReactor(); }

} // namespace tcp_echo::net

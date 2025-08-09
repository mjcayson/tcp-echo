#pragma once
#include <functional>

namespace tcp_echo::net {

using Fd = int;
using IoCallback = std::function<void(Fd)>;

class Reactor {
public:
  virtual ~Reactor() = default;
  virtual void add_read(Fd fd, IoCallback cb) = 0;
  virtual void del(Fd fd) = 0;
  virtual void run() = 0;
  virtual void stop() = 0;
};

} // namespace tcp_echo::net

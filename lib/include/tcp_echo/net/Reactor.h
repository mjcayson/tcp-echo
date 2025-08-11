#pragma once

#include <functional>

namespace tcp_echo::net
{
  using Fd = int;
  using IoCallback = std::function<void(Fd)>;

  class Reactor
  {
  public:
    virtual ~Reactor() = default;
    virtual void AddRead(Fd fd, IoCallback cb) = 0;
    virtual void Del(Fd fd) = 0;
    virtual void Run() = 0;
    virtual void Stop() = 0;
  };
} // namespace tcp_echo::net

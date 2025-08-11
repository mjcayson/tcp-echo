#include "tcp_echo/net/Reactor.h"
#include "tcp_echo/util/Logger.hpp"

#include <sys/select.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>

namespace tcp_echo::net
{
  class SelectReactor : public Reactor
  {
  public:
    void AddRead(Fd fd, IoCallback cb) override
    {
      m_callbacks[fd] = std::move(cb);
      if (fd > m_maxfd) m_maxfd = fd;
    }

    void Del(Fd fd) override
    {
      m_callbacks.erase(fd);
      if (fd == m_maxfd) RecomputeMax();
    }

    void SetTick(std::function<void()> tickCb, int intervalMs) override
    {
      m_tickCb = std::move(tickCb);
      m_intervalMs = intervalMs > 0 ? intervalMs : 0;
    }

    void Run() override
    {
      m_running = true;
      while (m_running)
      {
        fd_set rfds;
        FD_ZERO(&rfds);
        int localMaxfd = -1;
        for (auto& [fd, _] : m_callbacks)
        {
          FD_SET(fd, &rfds);
          if (fd > localMaxfd) localMaxfd = fd;
        }

        timeval tv{};
        timeval* ptv = nullptr;
        if (m_intervalMs > 0)
        {
          tv.tv_sec  = m_intervalMs / 1000;
          tv.tv_usec = (m_intervalMs % 1000) * 1000;
          ptv = &tv;
        }

        int rc = ::select(localMaxfd + 1, &rfds, nullptr, nullptr, ptv);
        if (rc < 0)
        {
          if (m_running) LOG_WARN("reactor", "select failed; continuing");
          continue;
        }
        else if (rc == 0)
        {
          if (m_tickCb != nullptr) m_tickCb();
          continue;
        }

        std::vector<int> ready;
        ready.reserve(static_cast<std::size_t>(rc));
        for (auto& [fd, _] : m_callbacks)
        {
          if (FD_ISSET(fd, &rfds)) ready.push_back(fd);
        }

        for (int fd : ready)
        {
          auto it = m_callbacks.find(fd);
          if (it != m_callbacks.end()) it->second(fd);
        }
      }
    }

    void Stop() override
    {
      m_running = false;
    }

  private:
    void RecomputeMax()
    {
      m_maxfd = -1;
      for (auto& [fd, _] : m_callbacks)
      {
        if (fd > m_maxfd) m_maxfd = fd;
      }
    }
    std::unordered_map<int, IoCallback> m_callbacks;
    int m_maxfd{-1};
    bool m_running{false};
    std::function<void()> m_tickCb{};
    int m_intervalMs{0};
  };

  // factory function for select reactor
  Reactor* MakeSelectReactor() { return new SelectReactor(); }
} // namespace tcp_echo::net

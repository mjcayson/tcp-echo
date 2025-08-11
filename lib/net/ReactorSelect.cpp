#include "tcp_echo/net/Reactor.h"
#include "tcp_echo/util/Logger.hpp"

#include <sys/select.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <algorithm>

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

        if (localMaxfd < 0)
        {
          //idle
          ::usleep(1000 * 10);
          continue;
        }

        int rc = ::select(localMaxfd + 1, &rfds, nullptr, nullptr, nullptr);
        if (rc < 0)
        {
          if (m_running) LOG_WARN("reactor", "select failed; continuing");
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
  };

  // factory function for select reactor
  Reactor* MakeSelectReactor() { return new SelectReactor(); }
} // namespace tcp_echo::net

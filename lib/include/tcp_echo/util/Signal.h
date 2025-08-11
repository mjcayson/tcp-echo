#pragma once

#include <atomic>

namespace tcp_echo::sys
{
  class StopToken
  {
  public:
    void RequestStop() { m_flag.store(true); }
    bool StopRequested() const { return m_flag.load(); }
  private:
    std::atomic<bool> m_flag{false};
  };

  void InstallSignalHandlers(StopToken& token);
} // namespace tcp_echo::sys

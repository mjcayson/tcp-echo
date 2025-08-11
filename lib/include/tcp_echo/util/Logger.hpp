#pragma once

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

namespace tcp_echo::log
{
  enum class Level
  {
    trace=0,
    debug,
    info,
    warn,
    error
  };

  class Logger
  {
  public:
    static Logger& instance()
    {
      static Logger inst;
      return inst;
    }

    void SetLevel(Level lvl)
    {
      m_level.store(static_cast<int>(lvl));
    }

    void write(Level lvl, const char* component, const std::string& msg)
    {
      if (static_cast<int>(lvl) < m_level.load()) return;
      const auto now = std::chrono::system_clock::now();
      const auto tt  = std::chrono::system_clock::to_time_t(now);
      const auto tid = std::this_thread::get_id();
      std::ostringstream oss;
      oss << std::put_time(std::gmtime(&tt), "%Y-%m-%dT%H:%M:%SZ");
      std::lock_guard<std::mutex> lock(m_mutex);
      std::cerr << oss.str() << " [" << LevelString(lvl) << "]"
                << " (" << component << ")"
                << " [tid=" << tid << "] "
                << msg << '\n';
    }

  private:
    Logger() = default;
    std::mutex m_mutex;
    std::atomic<int> m_level{static_cast<int>(Level::info)};

    static const char* LevelString(Level l) {
      switch (l)
      {
        case Level::trace:
          return "TRACE";
        case Level::debug:
          return "DEBUG";
        case Level::info:
          return "INFO";
        case Level::warn:
          return "WARN";
        case Level::error:
          return "ERROR";
        default:
          return "?";
      }
    }
  };

  inline void SetLevel(Level lvl)
  {
    Logger::instance().SetLevel(lvl);
  }

  inline void write(Level lvl, const char* comp, const std::string& msg)
  {
    Logger::instance().write(lvl, comp, msg);
  }
} // namespace tcp_echo::log

#define LOG_TRACE(comp, msg) ::tcp_echo::log::write(::tcp_echo::log::Level::trace, comp, (msg))
#define LOG_DEBUG(comp, msg) ::tcp_echo::log::write(::tcp_echo::log::Level::debug, comp, (msg))
#define LOG_INFO(comp,  msg) ::tcp_echo::log::write(::tcp_echo::log::Level::info,  comp, (msg))
#define LOG_WARN(comp,  msg) ::tcp_echo::log::write(::tcp_echo::log::Level::warn,  comp, (msg))
#define LOG_ERROR(comp, msg) ::tcp_echo::log::write(::tcp_echo::log::Level::error, comp, (msg))

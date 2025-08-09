#pragma once
#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

namespace tcp_echo::log {

enum class Level { trace=0, debug, info, warn, error };

class Logger {
public:
  static Logger& instance() {
    static Logger inst;
    return inst;
  }

  void set_level(Level lvl) { level_.store(static_cast<int>(lvl)); }

  void write(Level lvl, const char* component, const std::string& msg) {
    if (static_cast<int>(lvl) < level_.load()) return;
    const auto now = std::chrono::system_clock::now();
    const auto tt  = std::chrono::system_clock::to_time_t(now);
    const auto tid = std::this_thread::get_id();
    std::ostringstream ts;
    ts << std::put_time(std::gmtime(&tt), "%Y-%m-%dT%H:%M:%SZ");
    std::lock_guard<std::mutex> lk(mu_);
    std::cerr << ts.str() << " [" << level_str(lvl) << "]"
              << " (" << component << ")"
              << " [tid=" << tid << "] "
              << msg << '\n';
  }

private:
  Logger() = default;
  std::mutex mu_;
  std::atomic<int> level_{static_cast<int>(Level::info)};

  static const char* level_str(Level l) {
    switch (l) {
      case Level::trace: return "TRACE";
      case Level::debug: return "DEBUG";
      case Level::info:  return "INFO";
      case Level::warn:  return "WARN";
      case Level::error: return "ERROR";
    }
    return "?";
  }
};

inline void set_level(Level lvl) { Logger::instance().set_level(lvl); }
inline void write(Level lvl, const char* comp, const std::string& msg) {
  Logger::instance().write(lvl, comp, msg);
}

} // namespace tcp_echo::log

#define LOG_TRACE(comp, msg) ::tcp_echo::log::write(::tcp_echo::log::Level::trace, comp, (msg))
#define LOG_DEBUG(comp, msg) ::tcp_echo::log::write(::tcp_echo::log::Level::debug, comp, (msg))
#define LOG_INFO(comp,  msg) ::tcp_echo::log::write(::tcp_echo::log::Level::info,  comp, (msg))
#define LOG_WARN(comp,  msg) ::tcp_echo::log::write(::tcp_echo::log::Level::warn,  comp, (msg))
#define LOG_ERROR(comp, msg) ::tcp_echo::log::write(::tcp_echo::log::Level::error, comp, (msg))

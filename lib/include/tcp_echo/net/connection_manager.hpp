#pragma once
#include "tcp_echo/net/connection.hpp"
#include <unordered_map>

namespace tcp_echo::net {

class ConnectionManager {
public:
  void add(int fd, Connection&& c) { conns_.emplace(fd, std::move(c)); }
  void remove(int fd) { conns_.erase(fd); }
  bool empty() const { return conns_.empty(); }

private:
  std::unordered_map<int, Connection> conns_;
};

} // namespace tcp_echo::net

#pragma once

#include "tcp_echo/net/Connection.h"

#include <unordered_map>
#include <optional>

namespace tcp_echo::net
{
  class ConnectionManager
  {
  public:
    void Add(int fd, Connection&& c) { m_conns.emplace(fd, std::move(c)); }
    void Remove(int fd) { m_conns.erase(fd); }
    bool Empty() const { return m_conns.empty(); }

    Connection* GetConnection(int fd)
    {
      auto it = m_conns.find(fd);
      return (it == m_conns.end()) ? nullptr : &it->second;
    }

  private:
    std::unordered_map<int, Connection> m_conns;
  };
} // namespace tcp_echo::net

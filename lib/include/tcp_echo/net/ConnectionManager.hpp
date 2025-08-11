#pragma once

#include "tcp_echo/net/Connection.h"

#include <unordered_map>
#include <optional>

namespace tcp_echo::net
{
  class ConnectionManager
  {
  public:
    void Add(int fd, Connection&& conn) { m_conns.emplace(fd, std::move(conn)); }
    void Remove(int fd) { m_conns.erase(fd); }
    bool Empty() const { return m_conns.empty(); }

    Connection* GetConnection(int fd)
    {
      auto it = m_conns.find(fd);
      return (it == m_conns.end()) ? nullptr : &it->second;
    }

    template <typename Func>
    void ForEach(Func&& func)
    {
      for (auto& [fd, conn] : m_conns)
      {
        if (func(fd, conn) == false) break; // stop iteration if func returns false
      }
    }

  private:
    std::unordered_map<int, Connection> m_conns;
  };
} // namespace tcp_echo::net

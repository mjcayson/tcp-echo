#include "tcp_echo/net/Socket.h"
#include "tcp_echo/util/Logger.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace
{
  [[noreturn]] void SysThrow(const char* where)
  {
    throw std::runtime_error(std::string(where) + ": " + std::strerror(errno));
  }
}

namespace tcp_echo::net
{
  Socket::~Socket()
  {
    if (m_fd >= 0) ::close(m_fd);
  }

  Socket::Socket(Socket&& sock) noexcept
    : m_fd(sock.m_fd)
  {
    sock.m_fd = -1;
  }

  Socket& Socket::operator=(Socket&& sock) noexcept
  {
    if (this != &sock)
    {
      if (m_fd >= 0) ::close(m_fd);
      m_fd = sock.m_fd;
      sock.m_fd = -1;
    }

    return *this;
  }

  Socket Socket::CreateTcp()
  {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) SysThrow("socket");
    return Socket(fd);
  }

  void Socket::Close()
  {
    if (m_fd >= 0)
    {
      ::close(m_fd);
      m_fd = -1;
    }
  }

  void Socket::SetReuseAddr()
  {
    int opt = 1;
    if (::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
      SysThrow("setsockopt(SO_REUSEADDR)");
  }

  void Socket::SetNonblock(bool on)
  {
    int flags = ::fcntl(m_fd, F_GETFL, 0);
    if (flags < 0) SysThrow("fcntl(F_GETFL)");

    if (on) flags |= O_NONBLOCK;
    else flags &= ~O_NONBLOCK;

    if (::fcntl(m_fd, F_SETFL, flags) < 0) SysThrow("fcntl(F_SETFL)");
  }

  void Socket::BindAndListen(const std::string& host, uint16_t port, int backlog)
  {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1)
      throw std::runtime_error("inet_pton failed for host " + host);

    SetReuseAddr();
    if (::bind(m_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) SysThrow("bind");
    if (::listen(m_fd, backlog) < 0) SysThrow("listen");
  }

  Socket Socket::Accept(std::string* peerIP, uint16_t* peerPort)
  {
    sockaddr_in cli{};
    socklen_t len = sizeof(cli);
    int cfd = ::accept4(m_fd, reinterpret_cast<sockaddr*>(&cli), &len, SOCK_CLOEXEC);
    if (cfd < 0) SysThrow("accept");
    if (peerIP) {
      char buf[INET_ADDRSTRLEN];
      ::inet_ntop(AF_INET, &cli.sin_addr, buf, sizeof(buf));
      *peerIP = buf;
    }
    if (peerPort) *peerPort = ntohs(cli.sin_port);

    return Socket(cfd);
  }

  void Socket::Connect(const std::string& host, uint16_t port)
  {
    // Resolve hostname (e.g., "server" from docker-compose) to an IPv4 address
    addrinfo hints{};
    hints.ai_family   = AF_INET;       // IPv4 (Docker default bridge is IPv4)
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char portstr[16];
    std::snprintf(portstr, sizeof(portstr), "%u", static_cast<unsigned>(port));
    addrinfo* res = nullptr;

    int rc = ::getaddrinfo(host.c_str(), portstr, &hints, &res);
    if (rc != 0)
    {
      throw std::runtime_error(std::string("getaddrinfo: ") + ::gai_strerror(rc));
    }

    // Try the resolved addresses (should typically be one)
    for (addrinfo* ai = res; ai != nullptr; ai = ai->ai_next)
    {
      if (::connect(m_fd, ai->ai_addr, ai->ai_addrlen) == 0)
      {
        ::freeaddrinfo(res);
        return; // success
      }
    }

    ::freeaddrinfo(res);
    SysThrow("connect");
  }

  std::size_t Socket::SendAll(const void* data, std::size_t len)
  {
    const char* p = static_cast<const char*>(data);
    std::size_t sent = 0;
    while (sent < len)
    {
      ssize_t n = ::send(m_fd, p + sent, len - sent, 0);
      if (n < 0) SysThrow("send");
      sent += static_cast<std::size_t>(n);
    }

    return sent;
  }

  std::size_t Socket::RecvSome(void* buf, std::size_t len)
  {
    ssize_t n = ::recv(m_fd, buf, len, 0);
    if (n < 0) SysThrow("recv");

    return static_cast<std::size_t>(n);
  }

  std::size_t Socket::RecvExact(void* buf, std::size_t len)
  {
    std::size_t got = 0;
    while (got < len)
    {
      ssize_t numRecv = ::recv(m_fd, static_cast<char*>(buf) + got, len - got, 0);
      if (numRecv <= 0) SysThrow("recv");
      got += static_cast<std::size_t>(numRecv);
    }

    return got;
  }
} // namespace tcp_echo::net

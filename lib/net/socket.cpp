#include "tcp_echo/net/socket.hpp"
#include "tcp_echo/util/logger.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace tcp_echo::net {

namespace {
[[noreturn]] void sys_throw(const char* where) {
  throw std::runtime_error(std::string(where) + ": " + std::strerror(errno));
}
}

Socket::~Socket() { if (fd_ >= 0) ::close(fd_); }

Socket::Socket(Socket&& o) noexcept : fd_(o.fd_) { o.fd_ = -1; }

Socket& Socket::operator=(Socket&& o) noexcept {
  if (this != &o) {
    if (fd_ >= 0) ::close(fd_);
    fd_ = o.fd_;
    o.fd_ = -1;
  }
  return *this;
}

Socket Socket::create_tcp() {
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) sys_throw("socket");
  return Socket(fd);
}

void Socket::close() {
  if (fd_ >= 0) { ::close(fd_); fd_ = -1; }
}

void Socket::set_reuseaddr() {
  int opt = 1;
  if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    sys_throw("setsockopt(SO_REUSEADDR)");
}

void Socket::set_nonblock(bool on) {
  int flags = ::fcntl(fd_, F_GETFL, 0);
  if (flags < 0) sys_throw("fcntl(F_GETFL)");
  if (on) flags |= O_NONBLOCK; else flags &= ~O_NONBLOCK;
  if (::fcntl(fd_, F_SETFL, flags) < 0) sys_throw("fcntl(F_SETFL)");
}

void Socket::bind_and_listen(const std::string& host, uint16_t port, int backlog) {
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1)
    throw std::runtime_error("inet_pton failed for host " + host);

  set_reuseaddr();
  if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) sys_throw("bind");
  if (::listen(fd_, backlog) < 0) sys_throw("listen");
}

Socket Socket::accept(std::string* peer_ip, uint16_t* peer_port) {
  sockaddr_in cli{};
  socklen_t len = sizeof(cli);
  int cfd = ::accept4(fd_, reinterpret_cast<sockaddr*>(&cli), &len, SOCK_CLOEXEC);
  if (cfd < 0) sys_throw("accept");
  if (peer_ip) {
    char buf[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &cli.sin_addr, buf, sizeof(buf));
    *peer_ip = buf;
  }
  if (peer_port) *peer_port = ntohs(cli.sin_port);
  return Socket(cfd);
}

void Socket::connect(const std::string& host, uint16_t port) {
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1)
    throw std::runtime_error("inet_pton failed for host " + host);
  if (::connect(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) sys_throw("connect");
}

std::size_t Socket::send_all(const void* data, std::size_t len) {
  const char* p = static_cast<const char*>(data);
  std::size_t sent = 0;
  while (sent < len) {
    ssize_t n = ::send(fd_, p + sent, len - sent, 0);
    if (n < 0) sys_throw("send");
    sent += static_cast<std::size_t>(n);
  }
  return sent;
}

std::size_t Socket::recv_some(void* buf, std::size_t len) {
  ssize_t n = ::recv(fd_, buf, len, 0);
  if (n < 0) sys_throw("recv");
  return static_cast<std::size_t>(n);
}

std::size_t Socket::recv_exact(void* buf, std::size_t len) {
  std::size_t got = 0;
  while (got < len) {
    ssize_t n = ::recv(fd_, static_cast<char*>(buf) + got, len - got, 0);
    if (n <= 0) sys_throw("recv");
    got += static_cast<std::size_t>(n);
  }
  return got;
}

} // namespace tcp_echo::net

#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace tcp_echo::net {

class Socket {
public:
  Socket() = default;
  explicit Socket(int fd) noexcept : fd_(fd) {}
  ~Socket();

  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;
  Socket(Socket&& other) noexcept;
  Socket& operator=(Socket&& other) noexcept;

  static Socket create_tcp();
  void close();
  bool valid() const { return fd_ >= 0; }
  int  fd() const { return fd_; }

  void set_reuseaddr();
  void set_nonblock(bool on);

  void bind_and_listen(const std::string& host, uint16_t port, int backlog=128);
  Socket accept(std::string* peer_ip=nullptr, uint16_t* peer_port=nullptr);

  void connect(const std::string& host, uint16_t port);

  std::size_t send_all(const void* data, std::size_t len);
  std::size_t recv_some(void* buf, std::size_t len);
  std::size_t recv_exact(void* buf, std::size_t len);

private:
  int fd_{-1};
};

} // namespace tcp_echo::net

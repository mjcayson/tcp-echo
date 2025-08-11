#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace tcp_echo::net
{

  class Socket {
  public:
    Socket() = default;
    explicit Socket(int fd) noexcept
      : m_fd(fd) {}
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    static Socket CreateTcp();
    void Close();
    bool Valid() const { return m_fd >= 0; }
    int  fd() const { return m_fd; }

    void SetReuseAddr();
    void SetNonblock(bool on);

    void BindAndListen(const std::string& host, uint16_t port, int backlog=128);
    Socket Accept(std::string* peerIP=nullptr, uint16_t* peerPort=nullptr);

    void Connect(const std::string& host, uint16_t port);

    std::size_t SendAll(const void* data, std::size_t len);
    std::size_t RecvSome(void* buf, std::size_t len);
    std::size_t RecvExact(void* buf, std::size_t len);

  private:
    int m_fd{-1};
  };
} // namespace tcp_echo::net

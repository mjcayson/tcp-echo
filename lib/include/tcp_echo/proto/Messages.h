#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "tcp_echo/proto/Protocol.h"

namespace tcp_echo::proto
{
  // Fixed-size fields per spec
  constexpr std::size_t USER_LEN = 32;
  constexpr std::size_t PASS_LEN = 32;

  struct LoginRequest {
    std::string username; // will be padded to len 32
    std::string password; // will be padded to len 32
    uint8_t seq{};
  };

  struct LoginResponse {
    uint16_t status{1}; // 1=OK
    uint8_t  seq{};
  };

  struct EchoRequest {
    uint16_t msgSize{};
    std::vector<uint8_t> cipher; // size = msgSize
    uint8_t  seq{};
  };

  struct EchoResponse {
    uint16_t msgSize{};
    std::vector<uint8_t> plain; // size = msgSize
    uint8_t  seq{};
  };

  // serialization and deserialization
  std::vector<uint8_t> Serialize(const LoginRequest& req);
  bool Deserialize(const std::vector<uint8_t>& buf, LoginRequest& out);

  std::vector<uint8_t> Serialize(const LoginResponse& resp);
  bool Deserialize(const std::vector<uint8_t>& buf, LoginResponse& out);

  std::vector<uint8_t> Serialize(const EchoRequest& req);
  bool Deserialize(const std::vector<uint8_t>& buf, EchoRequest& out);

  std::vector<uint8_t> Serialize(const EchoResponse& resp);
  bool Deserialize(const std::vector<uint8_t>& buf, EchoResponse& out);
} // namespace tcp_echo::proto

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "tcp_echo/proto/protocol.hpp"

namespace tcp_echo::proto {

// Fixed-size fields per spec
constexpr std::size_t kUserLen = 32;
constexpr std::size_t kPassLen = 32;

struct LoginRequest {
  std::string username; // will be truncated/padded to 32
  std::string password; // "
  uint8_t     seq{};
};

struct LoginResponse {
  uint16_t status{1}; // 1=OK
  uint8_t  seq{};
};

struct EchoRequest {
  uint16_t msg_size{};
  std::vector<uint8_t> cipher; // size = msg_size
  uint8_t  seq{};
};

struct EchoResponse {
  uint16_t msg_size{};
  std::vector<uint8_t> plain; // size = msg_size
  uint8_t  seq{};
};

// (De)serialization
std::vector<uint8_t> serialize(const LoginRequest& req);
bool deserialize_login_req(const std::vector<uint8_t>& buf, LoginRequest& out);

std::vector<uint8_t> serialize(const LoginResponse& resp);
bool deserialize_login_resp(const std::vector<uint8_t>& buf, LoginResponse& out);

std::vector<uint8_t> serialize(const EchoRequest& req);
bool deserialize_echo_req(const std::vector<uint8_t>& buf, EchoRequest& out);

std::vector<uint8_t> serialize(const EchoResponse& resp);
bool deserialize_echo_resp(const std::vector<uint8_t>& buf, EchoResponse& out);

} // namespace tcp_echo::proto

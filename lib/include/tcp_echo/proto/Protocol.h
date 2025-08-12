#pragma once
#include <cstdint>

namespace tcp_echo::proto
{
  constexpr std::size_t HEADER_SIZE = 4; // size(2) + type(1) + seq(1)
  constexpr std::size_t MSG_SIZE_FIELD_SIZE = 2;

  enum class MsgType : uint8_t
  {
    LoginReq  = 0,
    LoginResp = 1,
    EchoReq   = 2,
    EchoResp  = 3
  };

  struct Header
  {
    uint16_t size; // includes header (big-endian)
    uint8_t  type;
    uint8_t  seq;
  };
} // namespace tcp_echo::proto

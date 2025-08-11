#pragma once

#include <cstdint>

namespace tcp_echo::byte_order
{
  inline uint16_t load_be16(const unsigned char* p)
  {
    return static_cast<uint16_t>((static_cast<uint16_t>(p[0]) << 8) | p[1]);
  }

  inline void store_be16(unsigned char* p, uint16_t v)
  {
    p[0] = static_cast<unsigned char>((v >> 8) & 0xFF);
    p[1] = static_cast<unsigned char>(v & 0xFF);
  }
} // namespace tcp_echo::byte_order

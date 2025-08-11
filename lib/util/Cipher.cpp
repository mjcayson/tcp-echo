#include "tcp_echo/util/Cipher.hpp"

#include <algorithm>

namespace tcp_echo::cipher
{
  uint8_t sum_u8(const std::string& s)
  {
    unsigned v = 0;
    for (unsigned char c : s) v = (v + c) & 0xFFu;
    return static_cast<uint8_t>(v);
  }

  uint32_t NextKey(uint32_t key)
  {
    return (key * 1103515245u + 12345u) % 0x7FFFFFFFu;
  }

  std::vector<uint8_t> Keystream(uint32_t initial, std::size_t n)
  {
    std::vector<uint8_t> ks;
    ks.reserve(n);
    uint32_t k = initial;
    for (std::size_t i = 0; i < n; ++i)
    {
      k = NextKey(k);
      ks.push_back(static_cast<uint8_t>(k % 256u));
    }

    return ks;
  }

  void InplaceXOR(std::vector<uint8_t>& data, const std::vector<uint8_t>& ks)
  {
    const std::size_t m = std::min(data.size(), ks.size());
    for (std::size_t i = 0; i < m; ++i) data[i] ^= ks[i];
  }
} // namespace tcp_echo::cipher

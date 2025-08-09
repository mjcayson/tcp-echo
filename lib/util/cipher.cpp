#include "tcp_echo/util/cipher.hpp"

namespace tcp_echo::cipher {

uint8_t sum_complement_u8(const std::string& s) {
  // Placeholder: ones' complement of uint8_t sum
  uint32_t sum = 0;
  for (unsigned char c : s) sum = (sum + c) & 0xFFu;
  return static_cast<uint8_t>(~sum);
}

uint32_t next_key(uint32_t key) {
  return (key * 1103515245u + 12345u) % 0x7FFFFFFFu;
}

std::vector<uint8_t> keystream(uint32_t initial, std::size_t n) {
  std::vector<uint8_t> ks;
  ks.reserve(n);
  uint32_t k = initial;
  for (std::size_t i = 0; i < n; ++i) {
    k = next_key(k);
    ks.push_back(static_cast<uint8_t>(k % 256u));
  }
  return ks;
}

void xor_inplace(std::vector<uint8_t>& data, const std::vector<uint8_t>& ks) {
  const std::size_t m = std::min(data.size(), ks.size());
  for (std::size_t i = 0; i < m; ++i) data[i] ^= ks[i];
}

} // namespace tcp_echo::cipher

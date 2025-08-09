#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace tcp_echo::cipher {

uint8_t sum_complement_u8(const std::string& s);        // to be verified with spec/example
uint32_t next_key(uint32_t key);                        // LCG
std::vector<uint8_t> keystream(uint32_t initial, std::size_t n);
void xor_inplace(std::vector<uint8_t>& data, const std::vector<uint8_t>& ks);

} // namespace tcp_echo::cipher

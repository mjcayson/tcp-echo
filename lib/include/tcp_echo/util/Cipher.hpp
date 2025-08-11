#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace tcp_echo::cipher
{
    // Sum of bytes modulo 256 (uint8_t overflow semantics).
    uint8_t sum_u8(const std::string& s);

    // LCG from the spec: (key*1103515245 + 12345) % 0x7FFFFFFF
    uint32_t NextKey(uint32_t key);

    // Produce n bytes of keystream where each byte is (state % 256).
    std::vector<uint8_t> Keystream(uint32_t initial, std::size_t n);

    // XOR data in-place with keystream bytes (min(len(data), len(ks))).
    void InplaceXOR(std::vector<uint8_t>& data, const std::vector<uint8_t>& ks);

    // initial_key = (seq << 16) | (userSum << 8) | passSum
    inline uint32_t MakeInitialKey(uint8_t seq, uint8_t userSum, uint8_t passSum)
    {
        return (static_cast<uint32_t>(seq) << 16) |
                (static_cast<uint32_t>(userSum) << 8) |
                static_cast<uint32_t>(passSum);
    }
} // namespace tcp_echo::cipher

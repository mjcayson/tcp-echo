#pragma once
#include <vector>
#include <cstdint>
#include "tcp_echo/proto/Protocol.h"

namespace tcp_echo::proto
{
    // Encodes [Header | Body] with BE16 size in Header
    std::vector<uint8_t> MakeFrame(MsgType type, uint8_t seq, const std::vector<uint8_t>& body);

    // Extract full frame from a growing buffer if available.
    // Returns empty vector if not enough data yet.
    std::vector<uint8_t> TryExtractFrame(std::vector<uint8_t>& inbuf);
} // namespace tcp_echo::proto

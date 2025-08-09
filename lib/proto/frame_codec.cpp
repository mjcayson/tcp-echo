#include "tcp_echo/proto/frame_codec.hpp"
#include "tcp_echo/util/byte_order.hpp"

namespace tcp_echo::proto {

std::vector<uint8_t> make_frame(MsgType type, uint8_t seq, const std::vector<uint8_t>& body) {
  std::vector<uint8_t> out;
  out.resize(kHeaderSize);
  out.insert(out.end(), body.begin(), body.end());
  bo::store_be16(out.data() + 0, static_cast<uint16_t>(out.size()));
  out[2] = static_cast<uint8_t>(type);
  out[3] = seq;
  return out;
}

std::vector<uint8_t> try_extract_frame(std::vector<uint8_t>& inbuf) {
  if (inbuf.size() < kHeaderSize) return {};
  const uint16_t size = bo::load_be16(inbuf.data());
  if (size == 0 || size > inbuf.size()) return {};
  std::vector<uint8_t> frame(inbuf.begin(), inbuf.begin() + size);
  inbuf.erase(inbuf.begin(), inbuf.begin() + size);
  return frame;
}

} // namespace tcp_echo::proto

#include "tcp_echo/proto/messages.hpp"
#include "tcp_echo/util/byte_order.hpp"
#include <algorithm>

namespace te = tcp_echo;
namespace bo = tcp_echo::bo;

namespace {

void pad_copy(std::vector<uint8_t>& out, const std::string& s, std::size_t n) {
  const auto take = std::min(n, s.size());
  out.insert(out.end(), s.begin(), s.begin() + static_cast<long>(take));
  out.insert(out.end(), n - take, 0);
}

} // namespace

namespace tcp_echo::proto {

std::vector<uint8_t> serialize(const LoginRequest& req) {
  std::vector<uint8_t> out;
  out.resize(kHeaderSize);
  // payload
  pad_copy(out, req.username, kUserLen);
  pad_copy(out, req.password, kPassLen);
  // header
  const uint16_t total = static_cast<uint16_t>(out.size());
  bo::store_be16(out.data() + 0, total);
  out[2] = static_cast<uint8_t>(MsgType::LoginReq);
  out[3] = req.seq;
  return out;
}

bool deserialize_login_req(const std::vector<uint8_t>& buf, LoginRequest& out) {
  if (buf.size() < kHeaderSize + kUserLen + kPassLen) return false;
  out.seq = buf[3];
  out.username.assign(reinterpret_cast<const char*>(buf.data() + 4), kUserLen);
  out.username.erase(out.username.find_first_of('\0'));
  out.password.assign(reinterpret_cast<const char*>(buf.data() + 4 + kUserLen), kPassLen);
  out.password.erase(out.password.find_first_of('\0'));
  return true;
}

std::vector<uint8_t> serialize(const LoginResponse& resp) {
  std::vector<uint8_t> out;
  out.resize(kHeaderSize + 2);
  bo::store_be16(out.data() + 0, static_cast<uint16_t>(out.size()));
  out[2] = static_cast<uint8_t>(MsgType::LoginResp);
  out[3] = resp.seq;
  bo::store_be16(out.data() + 4, resp.status);
  return out;
}

bool deserialize_login_resp(const std::vector<uint8_t>& buf, LoginResponse& outv) {
  if (buf.size() < kHeaderSize + 2) return false;
  outv.seq = buf[3];
  outv.status = bo::load_be16(buf.data() + 4);
  return true;
}

std::vector<uint8_t> serialize(const EchoRequest& req) {
  std::vector<uint8_t> out;
  out.resize(kHeaderSize + 2);
  // header placeholder; set later
  // body
  std::vector<uint8_t> body;
  body.resize(2);
  bo::store_be16(body.data(), req.msg_size);
  body.insert(body.end(), req.cipher.begin(), req.cipher.end());
  // stitch
  out.insert(out.end(), body.begin(), body.end());
  bo::store_be16(out.data() + 0, static_cast<uint16_t>(out.size()));
  out[2] = static_cast<uint8_t>(MsgType::EchoReq);
  out[3] = req.seq;
  return out;
}

bool deserialize_echo_req(const std::vector<uint8_t>& buf, EchoRequest& outv) {
  if (buf.size() < kHeaderSize + 2) return false;
  outv.seq = buf[3];
  outv.msg_size = bo::load_be16(buf.data() + 4);
  if (buf.size() < kHeaderSize + 2 + outv.msg_size) return false;
  outv.cipher.assign(buf.begin() + static_cast<long>(kHeaderSize + 2),
                     buf.begin() + static_cast<long>(kHeaderSize + 2 + outv.msg_size));
  return true;
}

std::vector<uint8_t> serialize(const EchoResponse& resp) {
  std::vector<uint8_t> out;
  out.resize(kHeaderSize + 2);
  std::vector<uint8_t> body;
  body.resize(2);
  bo::store_be16(body.data(), resp.msg_size);
  body.insert(body.end(), resp.plain.begin(), resp.plain.end());
  out.insert(out.end(), body.begin(), body.end());
  bo::store_be16(out.data() + 0, static_cast<uint16_t>(out.size()));
  out[2] = static_cast<uint8_t>(MsgType::EchoResp);
  out[3] = resp.seq;
  return out;
}

bool deserialize_echo_resp(const std::vector<uint8_t>& buf, EchoResponse& outv) {
  if (buf.size() < kHeaderSize + 2) return false;
  outv.seq = buf[3];
  outv.msg_size = bo::load_be16(buf.data() + 4);
  if (buf.size() < kHeaderSize + 2 + outv.msg_size) return false;
  outv.plain.assign(buf.begin() + static_cast<long>(kHeaderSize + 2),
                    buf.begin() + static_cast<long>(kHeaderSize + 2 + outv.msg_size));
  return true;
}

} // namespace tcp_echo::proto

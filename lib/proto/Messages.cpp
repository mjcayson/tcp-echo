#include "tcp_echo/proto/Messages.h"
#include "tcp_echo/util/ByteOrder.hpp"

#include <algorithm>

namespace te = tcp_echo;
namespace bo = tcp_echo::byte_order;

namespace
{
  void PadCopy(std::vector<uint8_t>& out, const std::string& s, std::size_t n)
  {
    const auto take = std::min(n, s.size());
    out.insert(out.end(), s.begin(), s.begin() + static_cast<long>(take));
    out.insert(out.end(), n - take, 0);
  }
} // namespace

namespace tcp_echo::proto
{
  std::vector<uint8_t> Serialize(const LoginRequest& req)
  {
    std::vector<uint8_t> out;
    out.resize(HEADER_SIZE);
    PadCopy(out, req.username, USER_LEN);
    PadCopy(out, req.password, PASS_LEN);
    const uint16_t total = static_cast<uint16_t>(out.size());
    byte_order::store_be16(out.data() + 0, total);
    out[2] = static_cast<uint8_t>(MsgType::LoginReq);
    out[3] = req.seq;
    return out;
  }

  bool Deserialize(const std::vector<uint8_t>& buf, LoginRequest& out)
  {
    if (buf.size() < HEADER_SIZE + USER_LEN + PASS_LEN) return false;
    out.seq = buf[3];
    { // username
      std::string str(reinterpret_cast<const char*>(buf.data() + 4), USER_LEN);
      auto pos = str.find('\0');
      if (pos != std::string::npos) str.resize(pos);
      out.username = std::move(str);
    }
    { // password
      std::string str(reinterpret_cast<const char*>(buf.data() + 4 + USER_LEN), PASS_LEN);
      auto pos = str.find('\0');
      if (pos != std::string::npos) str.resize(pos);
      out.password = std::move(str);
    }
    return true;
  }

  std::vector<uint8_t> Serialize(const LoginResponse& resp)
  {
    std::vector<uint8_t> out;
    out.resize(HEADER_SIZE + 2);
    byte_order::store_be16(out.data() + 0, static_cast<uint16_t>(out.size()));
    out[2] = static_cast<uint8_t>(MsgType::LoginResp);
    out[3] = resp.seq;
    byte_order::store_be16(out.data() + 4, resp.status);
    return out;
  }

  bool Deserialize(const std::vector<uint8_t>& buf, LoginResponse& outv)
  {
    if (buf.size() < HEADER_SIZE + 2) return false;
    outv.seq = buf[3];
    outv.status = byte_order::load_be16(buf.data() + 4);
    return true;
  }

  std::vector<uint8_t> Serialize(const EchoRequest& req)
  {
    std::vector<uint8_t> out;
    out.resize(HEADER_SIZE); //header only
    
    // body
    std::vector<uint8_t> body;
    body.resize(2);
    byte_order::store_be16(body.data(), req.msgSize);
    body.insert(body.end(), req.cipher.begin(), req.cipher.end());
    
    // stitch
    out.insert(out.end(), body.begin(), body.end());
    byte_order::store_be16(out.data() + 0, static_cast<uint16_t>(out.size()));
    out[2] = static_cast<uint8_t>(MsgType::EchoReq);
    out[3] = req.seq;
    return out;
  }

  bool Deserialize(const std::vector<uint8_t>& buf, EchoRequest& outv)
  {
    if (buf.size() < HEADER_SIZE + 2) return false;
    outv.seq = buf[3];
    outv.msgSize = byte_order::load_be16(buf.data() + 4);
    if (buf.size() < HEADER_SIZE + 2 + outv.msgSize) return false;
    outv.cipher.assign(buf.begin() + static_cast<long>(HEADER_SIZE + 2),
                       buf.begin() + static_cast<long>(HEADER_SIZE + 2 + outv.msgSize));
    return true;
  }

  std::vector<uint8_t> Serialize(const EchoResponse& resp)
  {
    std::vector<uint8_t> out;
    out.resize(HEADER_SIZE); //header only
    
    //body
    std::vector<uint8_t> body;
    body.resize(2);
    byte_order::store_be16(body.data(), resp.msgSize);
    body.insert(body.end(), resp.plain.begin(), resp.plain.end());
    
    //stitch
    out.insert(out.end(), body.begin(), body.end());
    byte_order::store_be16(out.data() + 0, static_cast<uint16_t>(out.size()));
    out[2] = static_cast<uint8_t>(MsgType::EchoResp);
    out[3] = resp.seq;
    return out;
  }

  bool Deserialize(const std::vector<uint8_t>& buf, EchoResponse& outv)
  {
    if (buf.size() < HEADER_SIZE + 2) return false;
    outv.seq = buf[3];
    outv.msgSize = byte_order::load_be16(buf.data() + 4);
    if (buf.size() < HEADER_SIZE + 2 + outv.msgSize) return false;
    outv.plain.assign(buf.begin() + static_cast<long>(HEADER_SIZE + 2),
                      buf.begin() + static_cast<long>(HEADER_SIZE + 2 + outv.msgSize));
    return true;
  }
} // namespace tcp_echo::proto

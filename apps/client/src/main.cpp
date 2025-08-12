#include "tcp_echo/util/Config.h"
#include "tcp_echo/util/Logger.hpp"
#include "tcp_echo/net/Socket.h"
#include "tcp_echo/proto/Messages.h"
#include "tcp_echo/util/ByteOrder.hpp"
#include "tcp_echo/util/Cipher.hpp"

#include <vector>
#include <string>

using namespace tcp_echo;

static std::vector<uint8_t> recv_frame(tcp_echo::net::Socket& sock)
{
  // read header
  unsigned char header[proto::HEADER_SIZE];
  sock.RecvExact(header, sizeof(header));
  const uint16_t size = byte_order::load_be16(header);
  std::vector<uint8_t> frame;
  frame.resize(size);
  // copy header
  frame[0] = header[0];
  frame[1] = header[1];
  frame[2] = header[2];
  frame[3] = header[3];
  // read remaining data
  if (size > proto::HEADER_SIZE)
  {
    sock.RecvExact(frame.data() + proto::HEADER_SIZE, size - proto::HEADER_SIZE);
  }

  return frame;
}

int main(int argc, char* argv[])
{
  auto cfg = ParseClientConfig(argc, argv);
  log::SetLevel(cfg.logLevel);

  net::Socket sock = net::Socket::CreateTcp();
  sock.Connect(cfg.host, cfg.port);
  LOG_INFO("client", "connected to " + cfg.host + ":" + std::to_string(cfg.port));

  // 1) Login (seq = 0)
  proto::LoginRequest loginReq{};
  loginReq.username = cfg.user;
  loginReq.password = cfg.pass;
  loginReq.seq = 0;
  auto lbytes = proto::Serialize(loginReq);
  sock.SendAll(lbytes.data(), lbytes.size());

  auto loginRespFrame = recv_frame(sock);
  proto::LoginResponse loginResp{};
  if (not proto::Deserialize(loginRespFrame, loginResp) || loginResp.status != 1)
  {
    LOG_ERROR("client", "login failed");
    return 1;
  }
  LOG_INFO("client", "login OK");

  // 2) Echo (seq = 1)
  const uint8_t seq = 1;
  // Encrypt message per spec (same as server decrypt)
  std::vector<uint8_t> plain(cfg.message.begin(), cfg.message.end());
  const uint8_t userSum = cipher::sum_u8(cfg.user);
  const uint8_t passSum = cipher::sum_u8(cfg.pass);
  const uint32_t init = cipher::MakeInitialKey(seq, userSum, passSum);
  auto kstream = cipher::Keystream(init, plain.size());
  std::vector<uint8_t> ciphered = plain;
  cipher::InplaceXOR(ciphered, kstream);

  proto::EchoRequest echoReq{};
  echoReq.seq = seq;
  echoReq.msgSize = static_cast<uint16_t>(ciphered.size());
  echoReq.cipher = std::move(ciphered);
  auto echoBytes = proto::Serialize(echoReq);
  sock.SendAll(echoBytes.data(), echoBytes.size());

  // 3) Read EchoResp and print message
  auto echoRespFrame = recv_frame(sock);
  proto::EchoResponse echoResp{};
  if (not proto::Deserialize(echoRespFrame, echoResp))
  {
    LOG_ERROR("client", "bad echo response");
    return 1;
  }

  std::string echoed{echoResp.plain.begin(), echoResp.plain.end()};
  LOG_INFO("client", "echoed: " + echoed);
  
  return 0;
}

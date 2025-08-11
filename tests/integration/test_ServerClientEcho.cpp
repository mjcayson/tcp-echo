#include "tcp_echo/util/ByteOrder.hpp"
#include "tcp_echo/util/Cipher.hpp"
#include "tcp_echo/net/Socket.h"
#include "tcp_echo/proto/Messages.h"

#include <gtest/gtest.h>
#include <cstdlib>
#include <string>
#include <vector>


using namespace tcp_echo;

static std::vector<uint8_t> RecvFrame(tcp_echo::net::Socket& sock)
{
  unsigned char header[proto::HEADER_SIZE];
  sock.RecvExact(header, sizeof(header));
  const uint16_t size = byte_order::load_be16(header + 0);
  std::vector<uint8_t> frame(size);

  // copy header
  frame[0]=header[0];
  frame[1]=header[1];
  frame[2]=header[2];
  frame[3]=header[3];
  if (size > proto::HEADER_SIZE)
  {
    sock.RecvExact(frame.data() + proto::HEADER_SIZE, size - proto::HEADER_SIZE);
  }

  return frame;
}

TEST(Integration, LoginAndEchoRoundTrip) {
  // Host/port come from env so this works in docker-compose (ECHO_HOST=server)
  const char* host = std::getenv("ECHO_HOST");
  const char* port = std::getenv("ECHO_PORT");
  const std::string HOST_NAME = host ? host : "server";
  const uint16_t    PORT_NUM = port ? static_cast<uint16_t>(std::stoi(port)) : 5000;

  const std::string user = "testuser";
  const std::string pass = "testpass";
  const std::string msg  = "hello integration";

  // Connect
  net::Socket sock = net::Socket::CreateTcp();
  sock.Connect(HOST_NAME, PORT_NUM);

  // 1) Login
  proto::LoginRequest loginReq{};
  loginReq.username = user;
  loginReq.password = pass;
  loginReq.seq = 0;
  auto loginBytes = proto::Serialize(loginReq);
  sock.SendAll(loginBytes.data(), loginBytes.size());

  auto loginRespFrame = RecvFrame(sock);
  proto::LoginResponse loginResp{};
  ASSERT_TRUE(proto::Deserialize(loginRespFrame, loginResp));
  ASSERT_EQ(loginResp.status, 1);
  ASSERT_EQ(loginResp.seq, 0);

  // 2) Echo (encrypt on client side per spec)
  const uint8_t seq = 1;
  std::vector<uint8_t> plain(msg.begin(), msg.end());
  const uint8_t us = cipher::sum_u8(user);
  const uint8_t ps = cipher::sum_u8(pass);
  const uint32_t init = cipher::MakeInitialKey(seq, us, ps);
  auto ks = cipher::Keystream(init, plain.size());
  std::vector<uint8_t> cipherText = plain;
  cipher::InplaceXOR(cipherText, ks);

  proto::EchoRequest echoReq{};
  echoReq.seq = seq;
  echoReq.msgSize = static_cast<uint16_t>(cipherText.size());
  echoReq.cipher = std::move(cipherText);
  auto echoReqBytes = proto::Serialize(echoReq);
  sock.SendAll(echoReqBytes.data(), echoReqBytes.size());

  auto echoRespFrame = RecvFrame(sock);
  proto::EchoResponse echoResp{};
  ASSERT_TRUE(proto::Deserialize(echoRespFrame, echoResp));
  ASSERT_EQ(echoResp.seq, seq);
  std::string echoed(echoResp.plain.begin(), echoResp.plain.end());
  EXPECT_EQ(echoed, msg);
}

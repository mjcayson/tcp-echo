#include "tcp_echo/util/ByteOrder.hpp"
#include "tcp_echo/proto/FrameCodec.h"
#include "tcp_echo/proto/Messages.h"

#include <gtest/gtest.h>
#include <vector>
#include <string>

using namespace tcp_echo;

namespace
{
  const std::size_t DEFAULT_MAX_FRAME = 8192;
}

TEST(Protocol, SunnyDay)
{
  proto::LoginRequest in{};
  in.seq = 7;
  in.username = "testuser";
  in.password = "testpass";

  auto bytes = proto::Serialize(in);

  proto::LoginRequest out{};
  ASSERT_TRUE(proto::Deserialize(bytes, out));
  EXPECT_EQ(out.seq, in.seq);
  EXPECT_EQ(out.username, in.username);
  EXPECT_EQ(out.password, in.password);

  // sanity: header size matches buffer length
  EXPECT_EQ(byte_order::load_be16(bytes.data()), bytes.size());
  EXPECT_EQ(bytes[2], static_cast<uint8_t>(proto::MsgType::LoginReq));
}

TEST(Protocol, UnknownMsgTypeFramePassesThroughCodec)
{
  // Craft a frame with unknown type=99 and 4-byte body
  const uint16_t total = proto::HEADER_SIZE + 4;
  std::vector<uint8_t> buf;
  buf.resize(total);
  byte_order::store_be16(buf.data(), total);
  buf[2] = 99;   // unknown type
  buf[3] = 7;    // seq
  //body
  buf[4] = 0xDE;
  buf[5] = 0xAD;
  buf[6] = 0xBE;
  buf[7] = 0xEF;

  //TryExtractFrame only cares about size, not semantic type
  auto tmp = buf;
  auto out = proto::TryExtractFrame(tmp, DEFAULT_MAX_FRAME);
  ASSERT_EQ(out.size(), total);
  ASSERT_TRUE(tmp.empty());    //already consumed
  EXPECT_EQ(out[2], 99);
  EXPECT_EQ(out[3], 7);
}

TEST(Protocol, EchoReqSizeMismatchRejectsDeserialize)
{
  // Build EchoReq | header | + | declared size=4 |, but only provide 2 bytes of cipher
  const uint16_t declaredCipherSize = 4;
  const uint16_t actualCipherSize = 2;
  const uint16_t total = proto::HEADER_SIZE + proto::MSG_SIZE_FIELD_SIZE + actualCipherSize;
  std::vector<uint8_t> buf{total, 0};
  byte_order::store_be16(buf.data(), total);
  buf[2] = static_cast<uint8_t>(proto::MsgType::EchoReq);
  buf[3] = 1; // seq

  // body: msgSize=4 but only append 2 bytes of cipher
  byte_order::store_be16(buf.data() + proto::HEADER_SIZE, declaredCipherSize);
  buf[6] = 0xAA;
  buf[7] = 0xBB; // missing 2 more bytes

  proto::EchoRequest req{};
  //Deserialize checks total length vs msgSize; this must fail.
  EXPECT_FALSE(proto::Deserialize(buf, req));
}

TEST(Protocol, ZeroTotalSizeThrows)
{
  std::vector<uint8_t> buf(proto::HEADER_SIZE);
  byte_order::store_be16(buf.data(), 0);
  buf[2] = 0;
  buf[3] = 0;
  EXPECT_THROW({
        auto _ = proto::TryExtractFrame(buf, DEFAULT_MAX_FRAME);
    }, std::runtime_error);
}

TEST(Protocol, LoginReqDeserializerRejectsWrongType)
{
    // Build a valid-sized frame but wrong type byte
    std::vector<uint8_t> buf(proto::HEADER_SIZE + proto::USER_LEN + proto::PASS_LEN, 0);
    byte_order::store_be16(buf.data(), static_cast<uint16_t>(buf.size()));
    buf[2] = 0xFF; // Invalid MsgType (not LoginReq)
    buf[3] = 42;   // seq

    proto::LoginRequest req{};
    EXPECT_FALSE(proto::Deserialize(buf, req));
}

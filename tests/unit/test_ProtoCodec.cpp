#include "tcp_echo/proto/FrameCodec.h"
#include "tcp_echo/util/ByteOrder.hpp"

#include <gtest/gtest.h>

using namespace tcp_echo;

namespace
{
  const std::size_t DEFAULT_MAX_FRAME = 8192;
}

TEST(FrameCodec, SunnyDay)
{
  // Craft a small valid frame: header+body=10 bytes
  const uint16_t total = 10;
  std::vector<uint8_t> buf;
  unsigned char header[proto::HEADER_SIZE];
  byte_order::store_be16(header, total);
  header[2] = 2; // echoReq
  header[3] = 1; // seq
  buf.insert(buf.end(), header, header + sizeof header);
  // body of size 6 (since header is 4)
  buf.insert(buf.end(), {1,2,3,4,5,6});

  auto frame = proto::TryExtractFrame(buf, DEFAULT_MAX_FRAME);
  ASSERT_EQ(frame.size(), total);
  EXPECT_TRUE(buf.empty()); // consumed
  EXPECT_EQ(frame[2], 2);
  EXPECT_EQ(frame[3], 1);
}

//test scenario where a client tries to send a frame larger than the maxFrame limit
TEST(FrameCodec, OversizeThrows)
{
  std::vector<uint8_t> buf;
  unsigned char header[proto::HEADER_SIZE];
  byte_order::store_be16(header, 2000); // 2000 bytes total
  header[2] = 0; // type
  header[3] = 0; // seq
  buf.insert(buf.end(), header, header + sizeof header);
  buf.resize(2000, 0); // pretend body present, inbuf has enough data

  EXPECT_THROW({
    auto tmp = proto::TryExtractFrame(buf, 1024);
  }, std::runtime_error);
}

//test scenario where a client sends a frame with size = 0
TEST(FrameCodec, ZeroSizeThrows)
{
  std::vector<uint8_t> buf;
  unsigned char header[proto::HEADER_SIZE];
  byte_order::store_be16(header, 0);  // illegal size
  header[2] = 0;
  header[3] = 0;
  buf.insert(buf.end(), header, header + sizeof header);

  EXPECT_THROW({
    auto frame = proto::TryExtractFrame(buf, 65535);
  }, std::runtime_error);
}

//test scenario where a client sends a frame with a missing body
TEST(FrameCodec, TruncatedReturnsEmpty)
{
  std::vector<uint8_t> buf;
  unsigned char header[proto::HEADER_SIZE];
  byte_order::store_be16(header, 100);
  header[2] = 0;
  header[3] = 0;
  buf.insert(buf.end(), header, header + sizeof(header));
  // body is missing
  auto frame = proto::TryExtractFrame(buf, DEFAULT_MAX_FRAME);
  EXPECT_TRUE(frame.empty());
  // buffer unchanged
  EXPECT_EQ(buf.size(), proto::HEADER_SIZE);
}

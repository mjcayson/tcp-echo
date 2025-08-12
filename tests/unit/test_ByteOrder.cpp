#include "tcp_echo/util/ByteOrder.hpp"

#include <gtest/gtest.h>

using namespace tcp_echo::byte_order;

TEST(ByteOrder, Be16RoundTrip)
{
  unsigned char buf[2];
  store_be16(buf, 0xABCD);
  EXPECT_EQ(buf[0], 0xAB);
  EXPECT_EQ(buf[1], 0xCD);
  EXPECT_EQ(load_be16(buf), 0xABCD);
}

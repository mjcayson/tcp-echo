#include "tcp_echo/util/Cipher.hpp"

#include <gtest/gtest.h>

using namespace tcp_echo;

TEST(Cipher, InitialKeyFromSums)
{
  // Example from spec: username "testuser", password "testpass", seq=87
  const uint8_t us = cipher::sum_u8("testuser"); // expect 0x7F
  const uint8_t ps = cipher::sum_u8("testpass"); // expect 0x77
  EXPECT_EQ(us, 0x7Fu);
  EXPECT_EQ(ps, 0x77u);
  const uint32_t init = cipher::MakeInitialKey(87, us, ps);
  EXPECT_EQ(init, 0x577F77u);
}

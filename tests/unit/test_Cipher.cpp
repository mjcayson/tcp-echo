#include "tcp_echo/util/Cipher.hpp"

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <string>

using namespace tcp_echo;

TEST(Cipher, InitialKeyFromSums)
{
  // Example from spec: username "testuser", password "testpass", seq=87
  const uint8_t userSum = cipher::sum_u8("testuser"); // expect 0x7F
  const uint8_t passSum = cipher::sum_u8("testpass"); // expect 0x77
  EXPECT_EQ(userSum, 0x7Fu);
  EXPECT_EQ(passSum, 0x77u);
  const uint8_t seq = 87;
  const uint32_t init = cipher::MakeInitialKey(seq, userSum, passSum);
  EXPECT_EQ(init, 0x577F77u);
}

TEST(Cipher, RandomRoundTrip)
{
  std::mt19937 rng(123456); // deterministic seed for test stability
  std::uniform_int_distribution<int> lenDist(0, 256);
  std::uniform_int_distribution<int> byteDist(0, 255);
  std::uniform_int_distribution<int> seqDist(0, 255);

  for (int tripNum = 0; tripNum < 10; ++tripNum)
  {
    const std::string user = "u" + std::to_string(tripNum);
    const std::string pass = "p" + std::to_string(tripNum);
    const uint8_t us = cipher::sum_u8(user);
    const uint8_t ps = cipher::sum_u8(pass);
    const uint8_t seq = static_cast<uint8_t>(seqDist(rng));
    const uint32_t init = cipher::MakeInitialKey(seq, us, ps);

    // random plaintext
    const int num = lenDist(rng);
    std::vector<uint8_t> plain(num);
    for (int counter = 0; counter < num; ++counter)
      plain[counter] = static_cast<uint8_t>(byteDist(rng));

    // encrypt
    auto ks = cipher::Keystream(init, plain.size());
    std::vector<uint8_t> cipher = plain;
    cipher::InplaceXOR(cipher, ks);

    // decrypt (same keystream)
    auto ks2 = cipher::Keystream(init, cipher.size());
    std::vector<uint8_t> recovered = cipher;
    cipher::InplaceXOR(recovered, ks2);

    EXPECT_EQ(recovered, plain);
  }
}
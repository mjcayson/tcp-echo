#include "tcp_echo/util/ByteOrder.hpp"
#include "tcp_echo/util/Cipher.hpp"
#include "tcp_echo/net/Socket.h"
#include "tcp_echo/proto/Messages.h"

#include <gtest/gtest.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <filesystem>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace tcp_echo;

static std::string GetEnv(const char* key, const std::string& defaultVal)
{
  const char* val = std::getenv(key);
  return val ? std::string(val) : defaultVal;
}

static uint16_t GetEnv(const char* key, int defaultVal)
{
  const char* val = std::getenv(key);
  return val ? static_cast<uint16_t>(std::stoi(val)) : static_cast<uint16_t>(defaultVal);
}

// Try connecting to host:port until it succeeds or we hit timeoutMs.
// Returns true if a TCP connect succeeded.
static bool WaitUntilListening(const std::string& host, uint16_t port, int timeoutMs)
{
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
  while (std::chrono::steady_clock::now() < deadline)
  {
    try
    {
      auto sock = net::Socket::CreateTcp();
      sock.Connect(host, port);
      //upon success, immediately close and return true
      return true;
    }
    catch (...)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }
  return false;
}

//spawn the server as a child process and return its pid
static pid_t SpawnServer(const std::string& host, uint16_t port, const std::string& reactor)
{
  std::string bin = "/build/apps/server/server"; //default path in Docker build

  pid_t pid = ::fork();
  if (pid < 0)
  {
    throw std::runtime_error("fork failed");
  }

  if (pid == 0)
  {
    // child: exec server
    std::string portStr = std::to_string(port);
    const char* argv[] = {
      bin.c_str(),
      "--host", host.c_str(),
      "--port", portStr.c_str(),
      "--reactor", reactor.c_str(),
      "--log-level", "warn",     // keep test output quiet
      nullptr
    };
    ::execv(argv[0], const_cast<char* const*>(argv));
    // if execv returns, it failed
    std::perror("execv");
    const int EXEC_FAILED = 127;
    _exit(EXEC_FAILED);
  }
  // parent: return child pid
  return pid;
}

static void TerminateProcess(pid_t pid, int graceMs = 2000)
{
  if (pid <= 0) return;
  //try to gracefully terminate the process with a SIGTERM
  ::kill(pid, SIGTERM);
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(graceMs);
  int status = 0;
  while (std::chrono::steady_clock::now() < deadline)
  {
    pid_t r = ::waitpid(pid, &status, WNOHANG);
    if (r == pid) return; // exited
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  // force kill if still running
  ::kill(pid, SIGKILL);
  ::waitpid(pid, &status, 0);
}

//read one frame from socket (blocking)
static std::vector<uint8_t> RecvFrameBlocking(tcp_echo::net::Socket& sock)
{
  unsigned char header[proto::HEADER_SIZE];
  sock.RecvExact(header, sizeof(header));
  const uint16_t size = byte_order::load_be16(header);
  std::vector<uint8_t> frame(size);
  // copy header
  frame[0] = header[0];
  frame[1] = header[1];
  frame[2] = header[2];
  frame[3] = header[3];
  if (size > proto::HEADER_SIZE)
  {
    sock.RecvExact(frame.data() + proto::HEADER_SIZE, size - proto::HEADER_SIZE);
  }
  return frame;
}

TEST(Integration, LoginAndEchoRoundTrip)
{
  // Choose host/port/reactor; allow env override
  std::string defaultHost{"127.0.0.1"};
  const std::string host = GetEnv("ECHO_HOST", defaultHost);
  auto defaultPort = 5500;
  const uint16_t port = GetEnv("ECHO_PORT", defaultPort);
  std::string defaultReactor{"select"};
  const std::string reactor = GetEnv("ECHO_REACTOR", defaultReactor);

  //start server child process
  pid_t serverPid = -1;
  ASSERT_NO_THROW(serverPid = SpawnServer(host, port, reactor));
  ASSERT_GT(serverPid, 0) << "server failed to spawn";

  //wait until server port is accepting connections
  auto timeoutMs = 3000;
  ASSERT_TRUE(WaitUntilListening(host, port, timeoutMs))
      << "server did not bind within timeout";

  //connect as a client and run protocol flow
  const std::string user = "testuser";
  const std::string pass = "testpass";
  const std::string msg  = "hello integration test";

  net::Socket sock = net::Socket::CreateTcp();
  sock.Connect(host, port);

  //login
  proto::LoginRequest loginReq{};
  loginReq.username = user;
  loginReq.password = pass;
  loginReq.seq = 0;
  auto loginBytes = proto::Serialize(loginReq);
  sock.SendAll(loginBytes.data(), loginBytes.size());

  auto loginRespFrame = RecvFrameBlocking(sock);
  proto::LoginResponse loginResp{};
  ASSERT_TRUE(proto::Deserialize(loginRespFrame, loginResp));
  ASSERT_EQ(loginResp.status, 1);
  ASSERT_EQ(loginResp.seq, 0);

  //generate ciphertext
  const uint8_t seq = 1;
  std::vector<uint8_t> plain(msg.begin(), msg.end());
  const uint8_t userSum = cipher::sum_u8(user);
  const uint8_t passSum = cipher::sum_u8(pass);
  const uint32_t init = cipher::MakeInitialKey(seq, userSum, passSum);
  auto ks = cipher::Keystream(init, plain.size());
  std::vector<uint8_t> cipher = plain;
  cipher::InplaceXOR(cipher, ks);

  //send echo request
  proto::EchoRequest echoReq{};
  echoReq.seq = seq;
  echoReq.msgSize = static_cast<uint16_t>(cipher.size());
  echoReq.cipher = std::move(cipher);
  auto echoBytes = proto::Serialize(echoReq);
  sock.SendAll(echoBytes.data(), echoBytes.size());

  //read echo response and verify contents
  auto echoRespFrame = RecvFrameBlocking(sock);
  proto::EchoResponse echoResp{};
  ASSERT_TRUE(proto::Deserialize(echoRespFrame, echoResp));
  ASSERT_EQ(echoResp.seq, seq);
  std::string echoed(echoResp.plain.begin(), echoResp.plain.end());
  EXPECT_EQ(echoed, msg);

  //shutdown server
  TerminateProcess(serverPid);
}

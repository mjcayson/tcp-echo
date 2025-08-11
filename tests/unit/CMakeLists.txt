file(GLOB_RECURSE UNIT_TESTS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp
)

#@TODO: Use the GLOB list later once all tests are ready 
add_executable(unit_tests
#   test_proto_codec.cpp
#   test_messages.cpp
  test_Cipher.cpp
  test_ByteOrder.cpp
#   test_config.cpp
#   test_signal.cpp
)

target_link_libraries(unit_tests
  PRIVATE
    tcp_echo_core
    GTest::gtest_main
)

#add the option to run: ctest -R unit
add_test(NAME unit COMMAND unit_tests)

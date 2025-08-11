add_executable(integration_tests
  test_ServerClientEcho.cpp
)

target_link_libraries(integration_tests
  PRIVATE
    tcp_echo_core
    GTest::gtest_main
)

#add the option to run: ctest -R integration
add_test(NAME integration COMMAND integration_tests)

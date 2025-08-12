file(GLOB_RECURSE UNIT_TESTS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp
)

add_executable(unit_tests
  ${UNIT_TESTS_SRC}
)

target_link_libraries(unit_tests
  PRIVATE
    tcp_echo_core
    GTest::gtest_main
)

#add the option to run: ctest -R unit
add_test(NAME unit COMMAND unit_tests)

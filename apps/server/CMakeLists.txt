add_executable(server src/main.cpp)
target_link_libraries(server PRIVATE tcp_echo_core)

add_executable(client src/main.cpp)
target_link_libraries(client PRIVATE tcp_echo_core)

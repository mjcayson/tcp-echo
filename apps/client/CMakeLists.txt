add_executable(client src/main.cpp src/client.cpp src/strategies.cpp)
target_link_libraries(client PRIVATE tcp_echo_core)

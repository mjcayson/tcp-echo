add_library(tcp_echo_core
    STATIC
        proto/protocol.cpp
        proto/messages.cpp
        proto/frame_codec.cpp
        net/socket.cpp
        net/reactor_select.cpp
        net/acceptor.cpp
        net/connection.cpp
        net/connection_manager.cpp
        net/server_core.cpp
        util/cipher.cpp
        util/byte_order.cpp
        util/config.cpp
        util/logger.cpp
        util/signal.cpp
        util/buffer.cpp
        util/timer.cpp
)

target_include_directories(tcp_echo_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)
target_link_libraries(tcp_echo_core PUBLIC Threads::Threads)

target_compile_definitions(tcp_echo_core PRIVATE _POSIX_C_SOURCE=200809L)

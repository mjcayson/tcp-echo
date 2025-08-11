file(GLOB_RECURSE TCP_ECHO_CORE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library(tcp_echo_core
    STATIC ${TCP_ECHO_CORE_SRC}
)

target_include_directories(tcp_echo_core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)
target_link_libraries(tcp_echo_core PUBLIC Threads::Threads)

target_compile_definitions(tcp_echo_core PRIVATE _POSIX_C_SOURCE=200809L)

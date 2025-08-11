cmake_minimum_required(VERSION 3.21)
project(tcp_echo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ECHO_BUILD_TESTS "Build unit/integration tests" OFF)
option(ECHO_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)
if (ECHO_WARNINGS_AS_ERRORS)
    add_compile_options(-Werror)
endif()

add_subdirectory(lib)
add_subdirectory(apps/server)
add_subdirectory(apps/client)

if (ECHO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

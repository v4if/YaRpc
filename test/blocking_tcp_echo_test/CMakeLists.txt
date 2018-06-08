cmake_minimum_required(VERSION 3.0)
project(blocking_tcp_echo_test)

set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++14")

find_package(Boost REQUIRED COMPONENTS system filesystem thread)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(blocking_tcp_echo_client blocking_client.cpp)
add_executable(blocking_tcp_echo_server blocking_server.cpp)

target_link_libraries(blocking_tcp_echo_client ${Boost_LIBRARIES})
target_link_libraries(blocking_tcp_echo_server ${Boost_LIBRARIES})

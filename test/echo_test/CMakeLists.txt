cmake_minimum_required(VERSION 3.0)
project(echo_test)

set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++14")

find_package(Boost REQUIRED COMPONENTS system filesystem thread)
include_directories(${Boost_INCLUDE_DIRS})

aux_source_directory(../../src core_src)
# aux_source_directory(${CMAKE_CURRENT_LIST_DIR} test_src)
add_executable(echo_server ${core_src} echo_server.cpp)
add_executable(echo_client ${core_src} echo_client.cpp)

target_link_libraries(echo_server ${Boost_LIBRARIES})
target_link_libraries(echo_client ${Boost_LIBRARIES})

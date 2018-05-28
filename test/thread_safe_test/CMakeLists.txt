cmake_minimum_required(VERSION 3.0)
project(thread_safe_test)

set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++0x -o0")

find_package(Boost REQUIRED COMPONENTS system filesystem thread)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(thread_safe_test thread_safe_test.cpp)

target_link_libraries(thread_safe_test ${Boost_LIBRARIES})

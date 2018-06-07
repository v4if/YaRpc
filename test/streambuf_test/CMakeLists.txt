cmake_minimum_required(VERSION 3.0)
project(streambuf_test)

set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++11")

find_package(Boost REQUIRED COMPONENTS system filesystem thread)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(streambuf_test streambuf_test.cpp)

target_link_libraries(streambuf_test ${Boost_LIBRARIES})

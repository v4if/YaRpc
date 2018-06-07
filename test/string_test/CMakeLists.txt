cmake_minimum_required(VERSION 3.0)
project(string_test)

set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++14")

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} test_src)
add_executable(string_test ${test_src})

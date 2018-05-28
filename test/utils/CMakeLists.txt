cmake_minimum_required(VERSION 3.0)
project(utils_test)

set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++14")

find_package(Boost REQUIRED COMPONENTS system filesystem thread)
include_directories(${Boost_INCLUDE_DIRS})

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} test_src)
add_executable(utils_test ${test_src})

target_link_libraries(utils_test ${Boost_LIBRARIES})

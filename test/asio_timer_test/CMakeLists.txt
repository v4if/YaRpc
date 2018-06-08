cmake_minimum_required(VERSION 3.0)
project(asio_timer_test)

set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++14")

find_package(Boost REQUIRED COMPONENTS system filesystem thread)
include_directories(${Boost_INCLUDE_DIRS})

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} test_src)
add_executable(${PROJECT_NAME} ${test_src})

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

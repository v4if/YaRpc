cmake_minimum_required(VERSION 3.0)
project(shared_ptr_test)

set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++14")

find_package (Threads)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR} test_src)
add_executable(${PROJECT_NAME} ${test_src})
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

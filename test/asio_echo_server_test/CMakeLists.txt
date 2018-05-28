cmake_minimum_required(VERSION 3.0)
project(asio_echo_server_test)

set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++14")

find_package(Boost REQUIRED COMPONENTS system filesystem thread)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(blocking_tcp_echo_client client.cc)
add_executable(blocking_tcp_echo_server echo_server.cc)

target_link_libraries(blocking_tcp_echo_client ${Boost_LIBRARIES})
target_link_libraries(blocking_tcp_echo_server ${Boost_LIBRARIES})


#include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
#link_directories(${CMAKE_CURRENT_LIST_DIR}/lib)
#aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src ${hello_src})
#add_executable(${PROJECT_NAME} ${hello_src})
#target_link_libraries(${PROJECT_NAME} util)
#set(CMAKE_CXX_COMPILER      "clang++" )         # 显示指定使用的C++编译器
#set(CMAKE_CXX_FLAGS   "-std=c++11")             # c++11
#set(CMAKE_CXX_FLAGS   "-g")                     # 调试信息
#set(CMAKE_CXX_FLAGS   "-Wall")                  # 开启所有警告
#set(CMAKE_CXX_FLAGS_DEBUG   "-O0" )             # 调试包不优化
#set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG " )   # release包优化
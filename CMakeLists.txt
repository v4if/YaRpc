cmake_minimum_required(VERSION 3.0)
project(yarpc)

set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++11")

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS echo.proto)

#find_package(Boost REQUIRED COMPONENTS system filesystem thread)
#include_directories(${Boost_INCLUDE_DIRS})

set(CORE_SRC src/server.cc)
set(CORE_HDR src/server.h)

add_executable(server ${CORE_SRC} ${CORE_HDR} server.cc ${PROTO_SRCS} ${PROTO_HDRS})
add_executable(client ${CORE_SRC} ${CORE_HDR} client.cc ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(server ${Protobuf_LIBRARIES})
target_link_libraries(client ${Protobuf_LIBRARIES})
#target_link_libraries(server ${Boost_LIBRARIES})
#target_link_libraries(client ${Boost_LIBRARIES})


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
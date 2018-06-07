cmake_minimum_required(VERSION 3.0)
project(pb_rpc_test)

set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++14")

find_package(Boost REQUIRED COMPONENTS system filesystem thread)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

protobuf_generate_cpp(Meta_src Meta_hdr ../../src/meta.proto)
protobuf_generate_cpp(Echo_src Echo_hdr echo.proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(../../src Core_Src)

add_executable(pb_client ${Core_Src} pb_client.cpp ${Meta_hdr} ${Meta_src} ${Echo_hdr} ${Echo_src})
add_executable(pb_server ${Core_Src} pb_server.cpp ${Meta_hdr} ${Meta_src} ${Echo_hdr} ${Echo_src})
target_link_libraries(pb_client ${Boost_LIBRARIES} ${Protobuf_LIBRARIES})
target_link_libraries(pb_server ${Boost_LIBRARIES} ${Protobuf_LIBRARIES})

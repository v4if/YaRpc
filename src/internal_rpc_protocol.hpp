#ifndef YARPC_INTERNAL_RPC_PROTOCOL_HPP_
#define YARPC_INTERNAL_RPC_PROTOCOL_HPP_

#include <iostream>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <boost/asio.hpp>
#include "controller.hpp"
#include "port.hpp"
#include "meta.pb.h"

typedef google::protobuf::MethodDescriptor gMethodDescriptor;
typedef google::protobuf::RpcController gController;
typedef google::protobuf::Message gMessage;
typedef google::protobuf::Closure gClosure;


namespace yarpc {

class server;

namespace internal_rpc_protocol {
    /**
     * |    uint32    |    uint32    |    data    |    data    |   uint32    |
     *     meta_size    request_size    meta_data  request_data  message_size
    */

    // Client
    // 序列化request，并添加meta
    void serialize_and_packed_request(std::string* buf_stream, const gMethodDescriptor *method, gController* controller, const gMessage* request);

    // server
    void process_and_unpacked_request(server* serv, boost::asio::streambuf& read_buff);

} // namespace internal_rpc_protocol

} // namespace internal_rpc_protocol



#endif

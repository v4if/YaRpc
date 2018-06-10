#ifndef YARPC_INTERNAL_RPC_PROTOCOL_HPP_
#define YARPC_INTERNAL_RPC_PROTOCOL_HPP_

#include <iostream>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <boost/asio.hpp>
#include "port.hpp"
#include "meta.pb.h"

typedef google::protobuf::MethodDescriptor gMethodDescriptor;
typedef google::protobuf::RpcController gController;
typedef google::protobuf::Message gMessage;
typedef google::protobuf::Closure gClosure;


namespace yarpc {

class Server;
namespace server_internal {
    class session;
}

namespace internal_rpc_protocol {
    /**
     * |    uint32    |    uint32    |    data    |    data    |   uint32    |
     *     meta_size    request_size req_meta_data  request_data  message_size
    */

    // client
    // 序列化request，并添加meta
    uint64 serialize_and_packed_request(std::string* buf_stream, 
        const gMethodDescriptor *method, 
        gController* controller, 
        const gMessage* request);

    void process_and_unpacked_response(boost::asio::streambuf& read_buff,
        google::protobuf::RpcController* controller);

    /**
     * |    uint32    |    uint32    |    data    |    data    |   uint32    |
     *     meta_size   response_size res_meta_data response_data  message_size
    */
    // server
    void serialize_and_packed_response(std::string* buf_stream, 
        gController* controller, 
        const gMessage* response);

    void process_and_unpacked_request(Server* serv, 
        std::weak_ptr<server_internal::session> session, 
        boost::asio::streambuf& read_buff);
} // namespace internal_rpc_protocol

} // namespace internal_rpc_protocol



#endif

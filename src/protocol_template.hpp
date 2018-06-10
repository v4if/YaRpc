#ifndef YARPC_PROTOCOL_TEMPLATE_HPP_
#define YARPC_PROTOCOL_TEMPLATE_HPP_

#include <iostream>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <boost/asio.hpp>
#include "controller.hpp"

namespace yarpc {

class Server;

struct protocol_template {
    typedef google::protobuf::MethodDescriptor gMethodDescriptor;
    typedef google::protobuf::RpcController gController;
    typedef google::protobuf::Message gMessage;
    typedef google::protobuf::Closure gClosure;


    // client
    std::function<uint64 (
        std::string* buf_stream, 
        const gMethodDescriptor *method, 
        gController* controller, 
        const gMessage* request)> 
    
    serialize_and_packed_request;


    std::function<void (
        boost::asio::streambuf& read_buff,
        google::protobuf::RpcController* controller)> 
        
    process_and_unpacked_response;

        
    // server
    std::function<void (
        std::string* buf_stream, 
        gController* controller, 
        const gMessage* response)>

    serialize_and_packed_response;


    std::function<void (
        Server* serv, 
        std::weak_ptr<server_internal::session> session, 
        boost::asio::streambuf& read_buff)>

    process_and_unpacked_request;
};

}

#endif

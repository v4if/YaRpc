#ifndef YARPC_PROTOCOL_TEMPLATE_HPP_
#define YARPC_PROTOCOL_TEMPLATE_HPP_

#include <iostream>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <boost/asio.hpp>
#include "controller.hpp"

namespace yarpc {

class server;

struct protocol_template {
    typedef google::protobuf::MethodDescriptor gMethodDescriptor;
    typedef google::protobuf::RpcController gController;
    typedef google::protobuf::Message gMessage;
    typedef google::protobuf::Closure gClosure;

    std::function<void(std::string* buf_stream, 
        const gMethodDescriptor *method, 
        gController* controller, 
        const gMessage* request)>
        serialize_and_packed_request;

    std::function<void(server* serv, boost::asio::streambuf& read_buff)>
        process_and_unpacked_request;
};

}

#endif

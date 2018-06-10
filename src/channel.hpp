#ifndef YARPC_CHANNEL_HPP_
#define YARPC_CHANNEL_HPP_

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <string>
#include "client.hpp"

typedef google::protobuf::MethodDescriptor gMethodDescriptor;
typedef google::protobuf::RpcController gController;
typedef google::protobuf::Message gMessage;
typedef google::protobuf::Closure gClosure;

namespace yarpc {

class Channel : public google::protobuf::RpcChannel {
public:
    Channel(Client* clt, char const* host, int const port);
    virtual ~Channel(); 
    virtual void CallMethod(const gMethodDescriptor *method, 
        gController *controller, 
        const gMessage *request, 
        gMessage *response, 
        gClosure *done);

private:
    Client* client_;
    char const* host_;
    int const port_;
};

} // namespace yarpc

#endif

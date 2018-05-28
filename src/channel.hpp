#ifndef yarpc_channel_hpp_
#define yarpc_channel_hpp_

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <string>
#include "client.hpp"

namespace yarpc {

using google::protobuf::Message;
using google::protobuf::Closure;
class Channel : public google::protobuf::RpcChannel {
public:
    Channel(Client *clt, char const* host, char const* port);
    virtual ~Channel(); 
    virtual void CallMethod(const google::protobuf::MethodDescriptor *method, google::protobuf::RpcController *controller, 
        const Message *request,Message *response, Closure *done);

private:
    Client* client_;
    
};

} // namespace yarpc

#endif

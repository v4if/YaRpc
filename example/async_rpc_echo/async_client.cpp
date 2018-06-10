#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include "../../src/client.hpp"
#include "../../src/channel.hpp"
#include "../../src/controller.hpp"
#include "echo.pb.h"

void HandleEchoResponse(
    yarpc::Controller* cntl,
    example::EchoResponse* response) {

    // std::unique_ptr makes sure cntl/response will be deleted before returning.
    std::unique_ptr<yarpc::Controller> cntl_guard(cntl);
    std::unique_ptr<example::EchoResponse> response_guard(response);

    if (cntl->Failed()) {
        return;
    }
    
    std::cout << response->message() << std::endl;
}

int main()
{
    yarpc::ClientOptions options;
    options.uptime = 5;
    yarpc::Client clt(options);
    yarpc::Channel channel(&clt, "127.0.0.1", 5678);

    clt.Start();

    example::EchoService_Stub stub(&channel);
    example::EchoRequest request;
    request.set_message("Hello world!");
 
    example::EchoResponse* response = new example::EchoResponse();
    yarpc::Controller* cntl = new yarpc::Controller();
 
    google::protobuf::Closure* done = 
        google::protobuf::NewCallback(&HandleEchoResponse, cntl, response);

    stub.Echo(cntl, &request, response, done);

    clt.Wait();
    return 0;
}

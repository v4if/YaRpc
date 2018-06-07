#include "../../src/client.hpp"
#include "../../src/channel.hpp"
#include "../../src/controller.hpp"
#include "echo.pb.h"

int main()
{
    yarpc::Client clt;
    yarpc::Channel channel(&clt, "127.0.0.1", 5678);

    example::EchoService_Stub stub(&channel);
    example::EchoRequest request;
    request.set_message("Hello world!");
 
    example::EchoResponse response;
 
    // sofa::pbrpc::RpcController controller;
    // controller.SetTimeout(3000);

    yarpc::Controller controller;
 
    clt.start();

    stub.Echo(&controller, &request, &response, NULL);
    stub.Hello(&controller, &request, &response, NULL);


    // if (controller.Failed()) {
    //     SLOG(ERROR, "request failed: %s", controller.ErrorText().c_str());
    // }

    clt.wait();
    return 0;
}
